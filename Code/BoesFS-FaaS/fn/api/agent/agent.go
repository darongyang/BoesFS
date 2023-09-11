package agent

import (
	"context"
	"errors"
	"fmt"
	"io"
	"net"
	"net/http"
	"os"
	"path/filepath"
	"sync"
	"time"
	"os/exec"
	"syscall"
	"regexp"

	"github.com/fnproject/fn/api/agent/gofs"

	"github.com/fnproject/fn/api/common"
	"github.com/fnproject/fn/api/models"
	"github.com/fnproject/fn/fnext"
	"github.com/sirupsen/logrus"
	"go.opencensus.io/plugin/ochttp"
	"go.opencensus.io/tag"
	"go.opencensus.io/trace"
	"go.opencensus.io/trace/propagation"
)

const (
	pauseTimeout = 5 * time.Second // docker pause/unpause
)

type Agent interface {
	// GetCall will return a Call that is executable by the Agent, which
	// can be built via various CallOpt's provided to the method.
	GetCall(...CallOpt) (Call, error)

	// Submit will attempt to execute a call locally, a Call may store information
	// about itself in its Start and End methods, which will be called in Submit
	// immediately before and after the Call is executed, respectively. An error
	// will be returned if there is an issue executing the call or the error
	// may be from the call's execution itself (if, say, the container dies,
	// or the call times out).
	Submit(Call) error

	// Close will wait for any outstanding calls to complete and then exit.
	// Closing the agent will invoke Close on the underlying DataAccess.
	// Close is not safe to be called from multiple threads.
	io.Closer

	AddCallListener(fnext.CallListener)
}

type agent struct {
	cfg           Config
	callListeners []fnext.CallListener

	// BoesFS-FaaS无需driver
	// driver drivers.Driver

	slotMgr *slotQueueMgr
	evictor Evictor
	// track usage
	resources ResourceTracker

	// used to track running calls / safe shutdown
	shutWg   *common.WaitGroup
	shutonce sync.Once

	// TODO(reed): shoot this fucking thing
	callOverrider CallOverrider

	// additional options to configure each call
	callOpts []CallOpt

	// deferred actions to call at end of initialisation
	onStartup []func()
}

// Option configures an agent at startup
type Option func(*agent) error

// RegistryToken is a reserved call extensions key to pass registry token
/* #nosec */
const RegistryToken = "FN_REGISTRY_TOKEN"

// New creates an Agent that executes functions locally as Docker containers.
func New(options ...Option) Agent {

	cfg, err := NewConfig()
	if err != nil {
		logrus.WithError(err).Fatalf("error in agent config cfg=%+v", cfg)
	}

	a := &agent{
		cfg: *cfg,
	}

	a.shutWg = common.NewWaitGroup()
	a.slotMgr = NewSlotQueueMgr()
	a.evictor = NewEvictor()

	// Allow overriding config
	for _, option := range options {
		err = option(a)
		if err != nil {
			logrus.WithError(err).Fatal("error in agent options")
		}
	}

	logrus.Infof("agent starting cfg=%+v", a.cfg)

	// BoesFS-FaaS无需driver

	a.resources = NewResourceTracker(&a.cfg)

	for _, sup := range a.onStartup {
		sup()
	}
	return a
}

func (a *agent) addStartup(sup func()) {
	a.onStartup = append(a.onStartup, sup)

}

// WithConfig sets the agent config to the provided config
func WithConfig(cfg *Config) Option {
	return func(a *agent) error {
		a.cfg = *cfg
		return nil
	}
}

// WithCallOverrider registers register a CallOverrider to modify a Call and extensions on call construction
func WithCallOverrider(fn CallOverrider) Option {
	return func(a *agent) error {
		if a.callOverrider != nil {
			return errors.New("lb-agent call overriders already exists")
		}
		a.callOverrider = fn
		return nil
	}
}

// WithCallOptions adds additional call options to each call created from GetCall, these
// options will be executed after any other options supplied to GetCall
func WithCallOptions(opts ...CallOpt) Option {
	return func(a *agent) error {
		a.callOpts = append(a.callOpts, opts...)
		return nil
	}
}


func (a *agent) Close() error {
	var err error

	// wait for ongoing sessions
	a.shutWg.CloseGroup()

	a.shutonce.Do(func() {
		// now close docker layer
		// BoesFS-FaaS无需driver
	})
	return err
}

func (a *agent) Submit(callI Call) error {
	// fmt.Println("\n\n\n---- agent.go Submit() start ----")
	call := callI.(*call)

	ctx := call.req.Context()

	// callID Key
	callIDKey, err := tag.NewKey("agent.call_id")
	if err != nil {
		return err
	}

	ctx, err = tag.New(ctx, tag.Insert(callIDKey, call.ID))
	if err != nil {
		return err
	}

	ctx, span := trace.StartSpan(ctx, "agent_submit")
	defer span.End()

	span.AddAttributes(
		trace.StringAttribute("fn.call_id", call.ID),
		trace.StringAttribute("fn.app_id", call.AppID),
		trace.StringAttribute("fn.fn_id", call.FnID),
	)
	rid := common.RequestIDFromContext(ctx)
	if rid != "" {
		span.AddAttributes(
			trace.StringAttribute("fn.rid", rid),
		)
	}

	// fmt.Printf("\n--- call:\n%+v \n\n", call)
	// fmt.Println("\n--- rid:\n", rid, "\n")
	// fmt.Println("\n--- call.ID:\n", call.ID, "\n")
	// fmt.Println("\n--- call.AppID:\n", call.AppID, "\n")
	// fmt.Println("\n--- call.FnID:\n", call.FnID, "\n")

	// fmt.Println("---- agent.go Submit() end ----\n\n\n")
	return a.submit(ctx, call)
}

func (a *agent) startStateTrackers(ctx context.Context, call *call) {
	call.requestState = NewRequestState()
}

func (a *agent) endStateTrackers(ctx context.Context, call *call) {
	call.requestState.UpdateState(ctx, RequestStateDone, call.slots)
}

func (a *agent) submit(ctx context.Context, call *call) error {
	// fmt.Println("\n\n\n---- agent.go submit() start ----")
	statsCalls(ctx)

	if !a.shutWg.AddSession(1) {
		statsTooBusy(ctx)
		return models.ErrCallTimeoutServerBusy
	}
	defer a.shutWg.DoneSession()

	statsEnqueue(ctx)

	a.startStateTrackers(ctx, call)
	defer a.endStateTrackers(ctx, call)


	// fmt.Printf("\n--- call:\n%+v \n\n", call)

	err := call.Start(ctx)
	if err != nil {
		return a.handleCallEnd(ctx, call, err, false)
	}

	statsDequeue(ctx)
	statsStartRun(ctx)

	// We are about to execute the function, set container Exec Deadline (call.Timeout)
	timeCtx, cancel := context.WithTimeout(ctx, time.Duration(call.Timeout)*time.Second)
	defer cancel()

	boesfsExec(timeCtx, call, &a.cfg)

	// fmt.Println("---- agent.go submit() end ----\n\n\n")
	return a.handleCallEnd(ctx, call, err, true)
}

func (a *agent) handleCallEnd(ctx context.Context, call *call, err error, isStarted bool) error {

	// This means call was routed (executed)
	if isStarted {
		call.End(ctx, err)
		statsStopRun(ctx)
		if err == nil {
			statsComplete(ctx)
		} else if err == context.DeadlineExceeded {
			statsTimedout(ctx)
			return models.ErrCallTimeout
		}
	} else {
		statsDequeue(ctx)
		if err == models.ErrCallTimeoutServerBusy || err == context.DeadlineExceeded {
			statsTooBusy(ctx)
			return models.ErrCallTimeoutServerBusy
		}
	}

	if err == context.Canceled {
		statsCanceled(ctx)
	} else if err != nil {
		statsErrors(ctx)
	}
	return err
}



func tryNotify(notifyChan chan error, err error) {
	if notifyChan != nil && err != nil {
		select {
		case notifyChan <- err:
		default:
		}
	}
}


// NameNotFoundError error for app not found when looked up by name
type CommandNotFoundError struct {
	Name string
}

func (n CommandNotFoundError) Error() string {
	return fmt.Sprintf("Command %s not found", n.Name)
}



func boesfsRunFDK(call *call, wg *sync.WaitGroup, respDone <-chan bool) error{
	fmt.Println("---- agent.go boesfsRunFDK() start ----\n\n\n")

	//TODO 
	// 0. 获取FUNCID
	// TODO
	// 1. 从~/.boesfs-faas/<FUNCID>目录下执行运行函数
	// 2. 设置env的sock文件为FUNCID

	funcCtx, cancel := context.WithCancel(context.Background())

	var err error


	// TODO 

	// 检查 fdk boesfs 环境
	_, err = exec.LookPath("fdk")
	if err != nil {
		fmt.Printf("fdk %s not found\n", "fdk")
		return CommandNotFoundError{"fdk"}
	} else {
		fmt.Printf("fdk %s found\n", "fdk")
	}

	var boesfsHave bool
	_, err = exec.LookPath("boesfs")
	if err != nil {
		fmt.Printf("boesfs %s not found\n", "boesfs")
		boesfsHave = false
	} else {
		fmt.Printf("boesfs %s found\n", "boesfs")
		boesfsHave = true
	}

	// 设置环境变量
	homeDir, _ := os.UserHomeDir()
	os.Setenv("LD_LIBRARY_PATH", homeDir+"/.boesfs/acl/library")

	fdkEnv := "env FDK_DEBUG=1 FN_FORMAT=http-stream " + "FN_LISTENER=unix:/"+"/tmp/"+call.ID+".sock "

	// filename := homeDir+"/.boesfs-faas/"+call.FnID+"/"+call.ID+".sock"
	filename := "/tmp/"+call.ID+".sock"
	// filename2 := homeDir+"/.boesfs-faas/"+call.FnID+"/"+"phony"+call.ID+".sock"
	filename2 := "/tmp/"+"phony"+call.ID+".sock"

	done := make(chan bool)
	fdkUp := make(chan bool)
	timeout := 1 * time.Second // 设置超时时间为1秒

	// 监听套接字文件是否建立
	go func() {
		startTime := time.Now()
		<- fdkUp // 等待fdk启动
	
		for {
			_, err = os.Stat(filename)
			if err == nil {
				done <- true
				return
			}
	
			if time.Since(startTime) >= timeout {
				return
			}
	
			time.Sleep( 10 * time.Millisecond) // 每10ms检查一次文件是否生成
		}
	}()

	// 如果有boesfs程序环境
	if boesfsHave {
		// boesfs -d / -k 0 <elf_name>
		// 标准版本的
		args := []string{"-d", "/home", "bash", "-c", fdkEnv + "fdk " + homeDir + "/.boesfs-faas/" + call.FnID + "/func.py " + "handler"}
		// args := []string{"-d", "/", "-k", "0", "bash", "-c", fdkEnv + "fdk " + homeDir + "/.boesfs-faas/" + call.FnID + "/func.py " + "handler"}
		// 旧版本的 待删除的
		// args := []string{"-a", "-d", "/", "-k1000", "bash", "-c", fdkEnv + "fdk "+ homeDir+"/.boesfs-faas/"+call.FnID+"/func.py " + "handler"}



		// 默认是python
		// TODO 添加其他语言
		// TODO 添加沙盒环境
		// TODO 用的二进制跑
		cmd := exec.CommandContext(funcCtx, "boesfs", args...)
		cmd.SysProcAttr = &syscall.SysProcAttr{Setpgid: true}

		cmd.Env = os.Environ()
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stderr

		err = cmd.Start()
		if err != nil {
			fmt.Println("启动fdk失败:", err)
			return err
		}

		fmt.Println("fdk已启动")
		fdkUp <- true

		// 函数运行结束后动态关闭fdk
		go func() {
			<- respDone
			// TODO 这里可以设置延缓几秒再关闭
			time.Sleep(60 * time.Second)
			cancel()
		}()

		select {
		case <-done:
			fmt.Println("套接字文件已生成")
			wg.Done()
		case <-time.After(timeout):
			fmt.Println("超时，文件未生成")
			return err
		}

		// 等待fdk执行完成
		err = cmd.Wait()

		// 检查命令是否被取消
		if funcCtx.Err() == context.Canceled {
			fmt.Println("函数运行结束，fdk关闭")
			err = syscall.Kill(-cmd.Process.Pid, syscall.SIGINT)
			if err != nil {
				fmt.Println("failed to Kill")
			}
		} else if err != nil {
			fmt.Println("fdk执行出错:", err)
		} else {
			fmt.Println("fdk自动退出")
		}
	} else {
	// 如果没有boesfs环境 则使用go项目
		// TODO
		// 调用gofs
		fdkDown := make(chan int)

		args := []string{"bash", "-c", fdkEnv + "fdk " + homeDir+"/.boesfs-faas/" + call.FnID + "/func.py " + "handler"}
		go func() {
			gofs.NonInteract(fdkDown, "/", "", 0, false, "~/.boesfs/acl/model/model.txt", "~/.boesfs/acl/model/policy.txt", "~/log.txt", args)
			fmt.Println("函数运行结束，fdk关闭")
		}()
		fdkUp <- true
		select {
		case <-done:
			fmt.Println("套接字文件已生成")
			wg.Done()
		case <-time.After(timeout):
			fmt.Println("超时，文件未生成")
			return err
		}

		<- respDone
		// TODO 这里可以设置延缓几秒再关闭
		time.Sleep(60 * time.Second)
		fdkDown <- 1
	}

	// 执行完成，删除文件
	err = os.Remove(filename)
	if err != nil {
		fmt.Println("清理套接字时出错:", err)
		return err
	}
	err = os.Remove(filename2)
	if err != nil {
		fmt.Println("清理套接字时出错:", err)
		return err
	}

	// 清除环境变量
	os.Unsetenv("LD_LIBRARY_PATH")

	fmt.Println("---- agent.go boesfsRunFDK() end ----\n\n\n")
	return err
}

func boesfsExec(ctx context.Context, call *call, cfg *Config) error{
	fmt.Println("\n\n\n---- agent.go boesfsExec() start ----")
	var wg sync.WaitGroup
	wg.Add(1)

	respDone := make(chan bool)

	// fmt.Printf("\n--- call:\n%+v \n\n", call)
	// fmt.Printf("\n--- ctx:\n%+v \n\n", ctx)

	// 不用这个是否可行
	// call.requestState.UpdateState(ctx, RequestStateExec, call.slots)

	call.req = call.req.WithContext(ctx) // TODO this is funny biz reed is bad
	
	fmt.Println("--- call.FnID:\n", call.FnID, "\n")

	// 正则
	regex := regexp.MustCompile(`phony.*\.sock`)

	homeDir, _ := os.UserHomeDir()

	files, err0 := os.ReadDir(homeDir+"/.boesfs-faas/"+call.FnID)
	if err0 != nil{
		fmt.Println("读取函数工作目录失败\n")
		return err0
	}

	sockFile := ""

	for _, file := range files{
		fmt.Println(file.Name())
		if regex.MatchString(file.Name()) {
			sockFile = homeDir+"/.boesfs-faas/"+call.FnID+ "/" + file.Name()
			fmt.Println(sockFile)
		}
	}

	var err error

	if sockFile != ""{
		fmt.Println("\n\n\n\n\n----------- 按道理不用重新生成fdk ------------ \n\n\n\n\n\n")
		fmt.Println("sockFile:\n", sockFile)
		// 直接调用 已存在的fdk sock
		err = boesfsDispatch(ctx, call, cfg, respDone, sockFile)
		// 如果成功万事大吉
		if err == nil {
			return err
		}
	}

	// 如果没成功 自己生成一个fdk来跑
	go boesfsRunFDK(call, &wg, respDone)
	wg.Wait()

	err = boesfsDispatch(ctx, call, cfg, respDone, "")

	fmt.Println("---- agent.go boesfsExec() end ----\n\n\n")
	return err
}



func createUDSRequest(ctx context.Context, call *call) *http.Request {
	req, err := http.NewRequest("POST", "http://localhost/call", call.req.Body)
	if err != nil {
		common.Logger(ctx).WithError(err).Error("somebody put a bad url in the call http request. 10 lashes.")
		panic(err)
	}
	// Set the context on the request to make sure transport and client handle
	// it properly and close connections at the end, e.g. when using UDS.
	req = req.WithContext(ctx)

	// remove transport headers before passing to function
	common.StripHopHeaders(call.req.Header)

	req.Header = make(http.Header)
	for k, vs := range call.req.Header {
		for _, v := range vs {
			req.Header.Add(k, v)
		}
	}

	req.Header.Set("Fn-Call-Id", call.ID)
	deadline, ok := ctx.Deadline()
	if ok {
		deadlineStr := deadline.Format(time.RFC3339)
		req.Header.Set("Fn-Deadline", deadlineStr)
	}

	return req
}


func boesfsDispatch(ctx context.Context, call *call, cfg *Config, respDone chan<- bool, nowSockFile string) error {
	fmt.Println("\n\n\n---- agent.go boesfsDispatch() start ----")
	// fmt.Printf("\n--- ctx:\n%+v \n\n", ctx)
	// fmt.Printf("\n--- call:\n%+v \n\n", call)

	req := createUDSRequest(ctx, call)
	req = req.WithContext(ctx)
	fmt.Printf("\n--- req:\n%+v \n\n", req)

	// homeDir, _ := os.UserHomeDir()


	sockFile := ""
	if nowSockFile == "" {
		// sockFile = homeDir+"/.boesfs-faas/"+call.FnID+"/"+call.ID+".sock"
		sockFile = "/tmp/"+call.ID+".sock"

	} else {
		sockFile = nowSockFile
	}

	baseTransport := &http.Transport{
		MaxIdleConns:           1,
		MaxIdleConnsPerHost:    1,
		MaxResponseHeaderBytes: int64(cfg.MaxHdrResponseSize),
		IdleConnTimeout:        1 * time.Second, // TODO(jang): revert this to 120s at the point all FDKs are known to be fixed
		// TODO(reed): since we only allow one, and we close them, this is gratuitous?
		DialContext: func(ctx context.Context, _, _ string) (net.Conn, error) {
			var d net.Dialer
			// return d.DialContext(ctx, "unix", filepath.Join(iofs.AgentPath(), udsFilename))
			return d.DialContext(ctx, "unix", sockFile)
		},
	}

	udsClient := http.Client{
		// use this transport so we can trace the requests to container, handy for debugging...
		Transport: &ochttp.Transport{
			NewClientTrace: ochttp.NewSpanAnnotatingClientTrace,
			Propagation:    noopOCHTTPFormat{}, // we do NOT want to send our tracers to user function, they default to b3
			Base:           baseTransport,
			// NOTE: the global trace sampler will be used, this is what we want for now at least
		},
	}

	var resp *http.Response
	var err2 error
	{ 
		resp, err2 = udsClient.Do(req)
		fmt.Printf("\n--- resp:\n%+v \n\n", resp)
		if err2 != nil {
			// TODO 套接字错误处理
			fmt.Println("\n--- err2:\n", err2, "\n")
			return err2
		}
		// 自己创建的 自己通知 借用的则不通知
		if nowSockFile == "" {
			respDone <- true
		}
	}

	defer resp.Body.Close()

	ioErrChan := make(chan error, 1)
	go func() {
		ioErrChan <- boesfsWriteResp(ctx, cfg.MaxResponseSize, resp, call.respWriter)
	}()

	fmt.Println("---- agent.go boesfsDispatch() end ----\n\n\n")

	select {
	case ioErr := <-ioErrChan:
		return ioErr
	case <-ctx.Done():
		if ctx.Err() == context.DeadlineExceeded {
			// TODO 移除hotSlot
			// s.SetError(ctx.Err())
		}
		return ctx.Err()
	}
}

func boesfsWriteResp(ctx context.Context, max uint64, resp *http.Response, w io.Writer) error {
	rw, ok := w.(http.ResponseWriter)
	if !ok {
		// TODO(reed): this is strange, we should just enforce the response writer type?
		w = common.NewClampWriter(w, max, models.ErrFunctionResponseTooBig)
		return resp.Write(w)
	}

	// IMPORTANT: Container contract: Enforce 200/502/504 expections
	switch resp.StatusCode {
	case http.StatusOK:
		// FDK processed the request OK
	case http.StatusBadGateway:
		// FDK detected failure, container can continue
		return models.ErrFunctionFailed
	case http.StatusGatewayTimeout:
		// FDK detected timeout, respond as if ctx expired, this gets translated & handled in handleCallEnd()
		return context.DeadlineExceeded
	default:
		// Any other code. Possible FDK failure. We shutdown the container

		// TODO hotSlot方法的替换
		// s.SetError(fmt.Errorf("FDK Error, invalid status code %d", resp.StatusCode))
		return models.ErrFunctionInvalidResponse
	}

	rw = newSizerRespWriter(max, rw)

	// remove transport headers before copying to client response
	common.StripHopHeaders(resp.Header)

	// WARNING: is the following header copy safe?
	// if we're writing directly to the response writer, we need to set headers
	// and only copy the body. resp.Write would copy a full
	// http request into the response body (not what we want).
	for k, vs := range resp.Header {
		for _, v := range vs {
			rw.Header().Add(k, v)
		}
	}
	rw.WriteHeader(http.StatusOK)

	_, ioErr := io.Copy(rw, resp.Body)
	return ioErr
}

// XXX(reed): this is a remnant of old io.pipe plumbing, we need to get rid of
// the buffers from the front-end in actuality, but only after removing other formats... so here, eat this
type sizerRespWriter struct {
	http.ResponseWriter
	w io.Writer
}

var _ http.ResponseWriter = new(sizerRespWriter)

func newSizerRespWriter(max uint64, rw http.ResponseWriter) http.ResponseWriter {
	return &sizerRespWriter{
		ResponseWriter: rw,
		w:              common.NewClampWriter(rw, max, models.ErrFunctionResponseTooBig),
	}
}

func (s *sizerRespWriter) Write(b []byte) (int, error) { return s.w.Write(b) }

// Attempt to queue/transmit the error to client
func runHotFailure(ctx context.Context, err error, caller slotCaller) {
	common.Logger(ctx).WithError(err).Info("hot function failure")
	tryNotify(caller.notify, err)
}

type runResult struct {
	err    error
	status string
}

func (r *runResult) Error() error   { return r.err }
func (r *runResult) Status() string { return r.status }


//checkSocketDestination verifies that the socket file created by the FDK is valid and permitted - notably verifying that any symlinks are relative to the socket dir
func checkSocketDestination(filename string) error {
	finfo, err := os.Lstat(filename)
	if err != nil {
		return fmt.Errorf("error statting unix socket link file %s", err)
	}

	if (finfo.Mode() & os.ModeSymlink) > 0 {
		linkDest, err := os.Readlink(filename)
		if err != nil {
			return fmt.Errorf("error reading unix socket symlink destination %s", err)
		}
		if filepath.Dir(linkDest) != "." {
			return fmt.Errorf("invalid unix socket symlink, symlinks must be relative within the unix socket directory")
		}
	}

	// stat the absolute path and check it is a socket
	absInfo, err := os.Stat(filename)
	if err != nil {
		return fmt.Errorf("unable to stat unix socket file %s", err)
	}
	if absInfo.Mode()&os.ModeSocket == 0 {
		return fmt.Errorf("listener file is not a socket")
	}

	return nil
}



var _ propagation.HTTPFormat = noopOCHTTPFormat{}

// we do not want to pass these to the user functions, since they're our internal traces...
// it is useful for debugging, admittedly, we could make it more friendly for OSS debugging...
type noopOCHTTPFormat struct{}

func (noopOCHTTPFormat) SpanContextFromRequest(req *http.Request) (sc trace.SpanContext, ok bool) {
	// our transport isn't receiving requests anyway
	return trace.SpanContext{}, false
}
func (noopOCHTTPFormat) SpanContextToRequest(sc trace.SpanContext, req *http.Request) {}


func cloneStrMap(src map[string]string) map[string]string {
	dst := make(map[string]string, len(src))
	for k, v := range src {
		dst[k] = v
	}
	return dst
}
