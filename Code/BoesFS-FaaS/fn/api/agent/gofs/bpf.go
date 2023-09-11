package gofs

/*
#cgo CFLAGS:
#cgo LDFLAGS: -L./modules -lboesfs -lelf -lcap -lpthread
// #cgo LDFLAGS: -L./modules 太奇怪了 需要和export LD_LIBRARY_PATH配合才能成功加载
// 获取错误信息
#include <errno.h>
#include <stdint.h>
extern int errno;
extern char *strerror (int __errnum) __THROW;
char* getErrorStr() {
	return strerror(errno);
}

// 加载bpf字节码
typedef struct ebpf_context {
        int ctrl_fd;
        int data_fd[32];
}ebpf_ctxtext_t;
extern ebpf_ctxtext_t* ebpf_init(char *filename, int index);
extern void reset_caps(void);
extern uint32_t hash_uint32(uint32_t integer);
extern uint32_t hash_str(char* str);
*/
import "C"
import (
	"bufio"
	"errors"
	"os"
	"strings"

	"github.com/cilium/ebpf"
	"go.uber.org/zap"
)

const (
	max_entries int = 32
)

type ebpf_context struct {
	CtrlFd int
	maps   [max_entries]*ebpf.Map
}

func (e ebpf_context) Close() (err error) {
	f := os.NewFile(uintptr(e.CtrlFd), "")
	err = f.Close()
	if err != nil {
		println("close ctrl_fd failed", err.Error())
		return err
	}

	for _, m := range e.maps {
		if m != nil {
			err = m.Close()
			if err != nil {
				println("close map failed", err.Error())
				return err
			}
		}
	}
	return nil
}

// LoadProgC uses dynamic linked library(.so) to load the bpf byte code
func LoadProgC(filePath string) (*ebpf_context, error) {
	logger := zap.L()
	ctx := C.ebpf_init(C.CString(filePath), 0)
	if ctx == nil {
		cErr := C.GoString(C.getErrorStr())
		if cErr == "Invalid argument" {
			logger.Fatal("load bpf failed, please confirm you have run `in.sh`", zap.String("CErrorString", cErr))
		} else {
			logger.Fatal("load bpf failed", zap.String("CErrorString", cErr))
		}
	}
	logger.Info("load bpf success", zap.Int("ctrl_fd", int(ctx.ctrl_fd)))
	// logger.Debug("fd", zap.Int("ctrl", int(ctx.ctrl_fd)), zap.Int("map", int(ctx.data_fd[0])))

	// Unmarshal value
	goCtx := ebpf_context{
		CtrlFd: int(ctx.ctrl_fd),
	}

	for idx, val := range ctx.data_fd {
		if val != 0 {
			// 设置map结构体
			// bpfMap, err := ebpf.NewMapFromFD(mapFd)
			// if err != nil {
			// 	// get map info: /proc/self/fdinfo/9: not supported 很可能是因为传错了fd
			// 	logger.Fatal("Error in NewMapFromFD, please notice your fd", zap.Error(err))
			// }

			// 如果ebpf.NewMapFromFD不可行, 可以尝试以下方案
			// mapInfo := ebpf.MapInfo{
			// 	Type:       ebpf.Hash,
			// 	KeySize:    uint32(PATH_MAX_SUPP),
			// 	ValueSize:  4,
			// 	MaxEntries: uint32(max_entries),
			// 	Flags:      0,
			// }
			// bpfMap, err := ebpf.NewMapFromFDAndInfo(mapFd, &mapInfo)

			m, err := ebpf.NewMapFromFD(int(val))
			if err != nil {
				logger.Error("NewMapFromFD", zap.Error(err))
				return nil, err
			}
			goCtx.maps[idx] = m
		}
	}

	// TODO: 可以将ctrl_fd用ebpf.NewProgramFromFD函数改为ebpf.Program, 目前没有这个需求
	return &goCtx, nil
}

// LoadProgC uses pure golang to load the bpf byte code
// 需要对库代码进行一定的修改, 以适应Boesfs类型
func LoadProgGo(filePath string) (*ebpf_context, error) {
	logger := zap.L()
	// Allow the current process to lock memory for eBPF resources.
	// 在内核完成
	// if err := rlimit.RemoveMemlock(); err != nil {
	// 	logger.Fatal("rlimit.RemoveMemlock", zap.Error(err))
	// }
	spec, err := ebpf.LoadCollectionSpec(filePath)
	if err != nil {
		logger.Fatal("LoadCollectionSpec", zap.Error(err))
	}
	logger.Debug("LoadCollectionSpec", zap.String("filePath", filePath))

	// TODO: 可以查看源仓库的issue, 可能可以不用指定tag
	var objs struct {
		BpfProg *ebpf.Program `ebpf:"boesfs_main_handler"`
		BpfMap  *ebpf.Map     `ebpf:"path_map"`
	}

	// Load pre-compiled programs and maps into the kernel.
	err = spec.LoadAndAssign(&objs, nil)
	if err != nil {
		logger.Fatal("LoadAndAssign", zap.Error(err))
	}
	defer objs.BpfProg.Close()
	defer objs.BpfMap.Close()
	logger.Debug("LoadAndAssign success")

	// // Get the first-mounted cgroupv2 path.
	// cgroupPath, err := detectCgroupPath()
	// if err != nil {
	// 	logger.Fatal("detectCgroupPath", zap.Error(err))
	// }
	// logger.Debug("detectCgroupPath")

	// // Link the count_egress_packets program to the cgroup.
	// l, err := link.AttachCgroup(link.CgroupOptions{
	// 	Path:    cgroupPath,
	// 	Attach:  ebpf.AttachCGroupInetEgress,
	// 	Program: objs.bpfProg,
	// })
	// if err != nil {
	// 	logger.Fatal("link.AttachCgroup", zap.Error(err))
	// }
	// defer l.Close()
	// logger.Debug("link.AttachCgroup")
	logger.Info("Load bpf success")

	ctx := ebpf_context{
		CtrlFd: objs.BpfProg.FD(),
		maps:   [max_entries]*ebpf.Map{objs.BpfMap},
	}
	return &ctx, nil
}

// detectCgroupPath returns the first-found mount point of type cgroup2
// and stores it in the cgroupPath global variable.
func detectCgroupPath() (string, error) {
	f, err := os.Open("/proc/mounts")
	if err != nil {
		return "", err
	}
	defer f.Close()

	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		// example fields: cgroup2 /sys/fs/cgroup/unified cgroup2 rw,nosuid,nodev,noexec,relatime 0 0
		fields := strings.Split(scanner.Text(), " ")
		if len(fields) >= 3 && fields[2] == "cgroup2" {
			return fields[1], nil
		}
	}

	return "", errors.New("cgroup2 not mounted")
}

// 设置ebpf map的值
// user mode的默认硬编码配置
func SetMapValue(bpfMap *ebpf.Map) {
	logger := zap.L()

	// 测试阶段使用硬编码值
	pathList := []string{
		"/home/boes/test/read.txt",
		"/home/boes/test/write.txt",
		"/home/boes/test/lookup.txt",
		"/home/boes/test/open.txt",
		"/home/boes/test/mkdir.txt",
		"/home/boes/test/unlink.txt",
		"/home/boes/test/rmdir.txt",
	}

	// 与map进行交互
	for idx, path := range pathList {
		// Update长度必须与定义的长度相同 详见`marshalers.go/func marshalBytes`
		pathBytes := make([]byte, bpfMap.KeySize())
		copy(pathBytes, path)

		err := bpfMap.Update(pathBytes, uint32(idx), ebpf.UpdateAny)
		if err != nil {
			logger.Fatal("map update failed", zap.String("path", path), zap.Error(err))
		}
		logger.Debug("map update", zap.String("path", path))

		// 检查map值, debug使用
		var value uint32
		err = bpfMap.Lookup(pathBytes, &value)
		if err != nil {
			logger.Fatal("map Lookup failed", zap.String("path", path), zap.Error(err))
		}
		logger.Debug("map get", zap.String("key", path), zap.Int("value", int(value)))

	}
	logger.Info("map update success")
}

func ResetCaps() {
	C.reset_caps()
}

func HashUint32(integer uint32) uint32 {
	return uint32(C.hash_uint32(C.uint32_t(integer)))
}

func HashStr(str string) uint32 {
	return uint32(C.hash_str(C.CString(str)))
}
