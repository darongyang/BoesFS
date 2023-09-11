package gofs

/*
// copyed from Code/BoesFS-Check-Module/acl/acl_public.h

#include <stdint.h>
#include <linux/types.h>

#define LINE_ARG_MAX 10

typedef struct line{
    __u32 nums;         //  参数个数
    __u32 args[LINE_ARG_MAX];
}line_t;

*/
import "C"
import (
	"fmt"
	"path"
	"regexp"
	"strconv"
	"strings"
	"unsafe"

	"github.com/cilium/ebpf"
	"go.uber.org/zap"
)

// 读取ACL模型并设置map的值
// ACL mode
// argv0: 目标可执行文件的路径
func ReadAclAndSetMapValue(ctx *ebpf_context, argv0 string, modelPath string, policyPath string) {
	logger := zap.L()

	// TODO: 可以把前面解析model的部分拆开, 不然变量太多, 可能会互相影响, 看的也不舒服
	// 打开model和policy定义文件
	logger.Info("path", zap.String("model", ParseHome(modelPath)), zap.String("policy", ParseHome(policyPath)))
	model, err := OpenAndReadAllContent(modelPath)
	if err != nil {
		logger.Fatal("OpenAndReadAllContent", zap.String("modelPath", modelPath), zap.Error(err))
	}
	policy, err := OpenAndReadAllContent(policyPath)
	if err != nil {
		logger.Fatal("OpenAndReadAllContent", zap.String("policyPath", policyPath), zap.Error(err))
	}

	/********** 读取model **********/
	// 定义一些变量 后面无需 :=
	var match []string
	var value string
	var re *regexp.Regexp

	// request_definition
	// 没有实际用处, 暂时忽略

	// policy_definition
	re = regexp.MustCompile(`\[policy_definition\]\s*p\s*=\s*(.*)`)
	match = re.FindStringSubmatch(model)
	if match == nil && len(match) != 2 {
		logger.Fatal("match [policy_definition] failed")
	}
	// 后续只用到了policy_def_map
	value = match[1]                                       // p = (这一部分), 一直匹配到行尾
	policies := []string{"sub", "obj", "act", "args"}      // 所有policy种类
	policy_def_map := make(map[string]bool, len(policies)) // 每种policy是否被定义
	for _, policy := range policies {
		re = regexp.MustCompile(policy)
		policy_def_map[policy] = re.MatchString(value)
	}
	// eBPF程序没用到这个map, 暂时不进行交互

	// role_definition
	// BUGS 目前如果需要二者同时存在, 需要写两次role_definition
	re = regexp.MustCompile(`\[role_definition\]\s*g\s*=\s*_\s*,\s*_`) // g = _ , _
	is_rbac_sub := re.MatchString(model)
	re = regexp.MustCompile(`\[role_definition\]\s*a\s*=\s*_\s*,\s*_\s*;\s*_\s*,\s*_`) // a = _ , _ ; _ , _
	is_rbac_act_args := re.MatchString(model)

	// policy_effect
	re = regexp.MustCompile(`\[policy_effect\]\s*e\s*=\s*(.*)`)
	match = re.FindStringSubmatch(model)
	if match == nil && len(match) != 2 {
		logger.Fatal("match [policy_effect] failed")
	}
	value = match[1] // e = (这一部分)
	// 构造结构体
	policy_eff := C.line_t{}
	re = regexp.MustCompile(`!`)
	if re.MatchString(value) {
		policy_eff.args[0] = EFFECT_DENY
	} else {
		policy_eff.args[0] = EFFECT_ALLOW
	}
	policy_eff.nums = 1
	err = ctx.maps[0].Update(uint32(POL_EFF_IDX), policy_eff, ebpf.UpdateAny)
	if err != nil {
		logger.Fatal("update policy_effect", zap.Error(err))
	}

	// matchers
	re = regexp.MustCompile(`\[matchers\]\s*m\s*=\s*(.*)`)
	match = re.FindStringSubmatch(model)
	if match == nil && len(match) != 2 {
		logger.Fatal("match [matchers] failed")
	}
	value = match[1] // m = (这一部分)
	// 构造结构体
	// 将matchers改造为bitmap, 为了和c那边保持一致, 下标小的在bitmap的低位
	match_result := 0 // 设置一个bitmap, 方便后续选择
	for i, m := range policies {
		if !policy_def_map[m] {
			continue
		}
		re = regexp.MustCompile(fmt.Sprintf(`r.%s\s*==\s*p.%s`, m, m))
		if re.MatchString(value) {
			match_result |= 1 << i
		}
	}

	// 对一些特定的match选项进行处理
	// rabc
	re = regexp.MustCompile(`g\(r\.sub\s*,\s*p\.sub\)`)
	if re.MatchString(value) {
		// sub和rabc sub冲突
		if match_result&1 != 0 {
			logger.Fatal("r.sub==p.sub conflict with g(r.sub,p.sub)")
		}

		if !is_rbac_sub {
			logger.Warn("rabc: set g(r.sub,p.sub) but no role_definition")
		}
		match_result |= 1
	} else if is_rbac_sub {
		is_rbac_sub = false
		logger.Warn("rabc: set role_definition but no g(r.sub,p.sub)")
	}
	// args
	is_args := false
	if match_result&8 != 0 {
		is_args = true
	}
	// is_rbac_act_args
	re = regexp.MustCompile(`a(r.act,p.act;r.args,p.args)`)
	if policy_def_map["act"] && policy_def_map["args"] && is_rbac_act_args && re.MatchString(value) {
		// act_args和rabc act_args冲突
		if match_result&4 != 0 || match_result&8 != 0 {
			logger.Fatal("r.act/args==p.act/args conflict with a(r.act,p.act;r.args,p.args)")
		}
		match_result |= 12
		is_args = true
		if !is_rbac_sub {
			logger.Warn("rabc: set a(r.act,p.act;r.args,p.args) but no role_definition")
		}
	} else if is_rbac_act_args {
		is_rbac_sub = false
		logger.Warn("rabc: set role_definition but no a(r.act,p.act;r.args,p.args)")
	}
	logger.Info("match", zap.Int("match_result", match_result))

	matchers := C.line_t{}
	switch match_result {
	case 3:
		matchers.args[0] = SUB_OBJ
	case 5:
		matchers.args[0] = SUB_ACT
		// TODO: 不支持SUB_ACT, 因为其逻辑较为复杂, 且可以使用SUB_OBJ_ACT通过`[sub], /, [act], dir`代替
		logger.Fatal("not support SUB_ACT, please use SUB_OBJ_ACT( [sub], /, [act], dir )")
	case 6:
		matchers.args[0] = OBJ_ACT
	case 7:
		matchers.args[0] = SUB_OBJ_ACT
	case 14:
		matchers.args[0] = OBJ_ACT_ARGS
	case 15:
		matchers.args[0] = SUB_OBJ_ACT_ARGS
	default:
		logger.Fatal("wrong match_result", zap.Int("match_result", match_result))
	}
	matchers.nums = 1
	err = ctx.maps[0].Update(uint32(MATCHER_IDX), matchers, ebpf.UpdateAny)
	if err != nil {
		logger.Fatal("update matchers", zap.Error(err))
	}
	logger.Info("model",
		zap.Int("matchers.args", int(matchers.args[0])),
		zap.Bool("is_args", is_args),
		zap.Bool("is_rbac_sub", is_rbac_sub),
		zap.Bool("is_rbac_act_args", is_rbac_act_args),
	)

	logger.Info("analyse model success")

	/********** 读取policy **********/
	// 读取分组情况
	re = regexp.MustCompile(`g\s*,\s*([\w/.]+)\s*,\s*([\w/.]+)`)
	validGroups := []string{} // 目标可执行文件所在的所有组
	for _, group := range re.FindAllStringSubmatch(policy, -1) {
		binName := group[1]
		groupName := group[2]
		// BUGS: 与运行的程序*名称*一样则算作一组
		if path.Base(argv0) == path.Base(binName) {
			validGroups = append(validGroups, groupName)
		}
	}

	// 读取act_args定义
	re = regexp.MustCompile(`a\s*,\s*([\w/.]+)\s*,\s*([\w/.]+)\s*,\s*\((.+?)\)`)
	type actArgs struct {
		actOp   int
		argHash uint32 // 这里提前hash, 是为了后续优化考虑
	}
	actArgsMap := make(map[string][]actArgs, 0)
	for _, match := range re.FindAllStringSubmatch(policy, -1) {
		groupName := match[1]
		act := match[2]
		args := match[3]
		op := checkOp(act)
		if op == -1 {
			logger.Fatal("invalid policy act group", zap.String("policy", match[0]), zap.String("act", act))
		}
		actArgsMap[groupName] = append(actArgsMap[groupName], actArgs{op, hashArgs(args)})
	}

	// 这里直接全部读取并使用正则表达式进行全匹配, 而不是一行一行读取
	// 主要是考虑到目前的policy较少, IO效率可能比正则表达式低
	// BUGS 这里的matchers和policy_def是混用的, 我也分不清楚, 很多地方可能有问题
	reString := `p`
	switch matchers.args[0] {
	case SUB_OBJ, SUB_ACT, OBJ_ACT, OBJ_ACT_ARGS:
		// 两个匹配项
		reString += `\s*,\s*([\w/.]+)\s*,\s*([\w/.]+)`
	case SUB_OBJ_ACT, SUB_OBJ_ACT_ARGS:
		// 三个匹配项
		reString += `\s*,\s*([\w/.]+)\s*,\s*([\w/.]+)\s*,\s*([\w/.]+)`
	default:
		logger.Fatal("wrong matchers")
	}
	if is_args && !is_rbac_act_args {
		// 需要加一个匹配args的
		reString += `\s*,\s*\((.+?)\)`
	}
	reString += `\s*,\s*(file|dir)\s*,\s*(allow|deny)`
	re = regexp.MustCompile(reString)
	matches := re.FindAllStringSubmatch(policy, -1)
	if matches == nil {
		logger.Fatal("re.FindAllStringSubmatch(policy, -1)", zap.String("reString", reString))
	}

	dirCnt := 0 // 已识别的dir规则的数量, 用于确定dir map的index
	for _, match := range matches {
		// 首先将正则匹配项进行解析
		// 暂时通过matchers解析, 因为我也分不清matchers和policy_def有什么不同的作用

		// 如果is_rbac_act_args为true, 则args是args的组名
		var sub, obj, act, args, file_dir, allow_deny string
		switch matchers.args[0] {
		case SUB_OBJ_ACT:
			sub, obj, act, file_dir, allow_deny = match[1], match[2], match[3], match[4], match[5]
		case SUB_OBJ:
			sub, obj, file_dir, allow_deny = match[1], match[2], match[3], match[4]
		case SUB_ACT:
			// sub, act, file_dir, allow_deny = match[1], match[2], match[3], match[4]
			logger.Fatal("not support SUB_ACT, please use SUB_OBJ_ACT( [sub], /, [act], dir )")
		case OBJ_ACT:
			obj, act, file_dir, allow_deny = match[1], match[2], match[3], match[4]
		case SUB_OBJ_ACT_ARGS:
			if is_rbac_act_args {
				sub, obj, args, file_dir, allow_deny = match[1], match[2], match[3], match[4], match[5]
			} else {
				sub, obj, act, args, file_dir, allow_deny = match[1], match[2], match[3], match[4], match[5], match[6]
			}
		case OBJ_ACT_ARGS:
			if is_rbac_act_args {
				obj, args, file_dir, allow_deny = match[1], match[2], match[3], match[4]
			} else {
				obj, act, args, file_dir, allow_deny = match[1], match[2], match[3], match[4], match[5]
			}
		}

		// BUGS: 将sub组名简单地替换为可执行文件名, 方便后续处理
		// 其实到这一步, rbac_sub的逻辑就已经完成了, 就是单纯的替换
		if is_rbac_sub {
			for _, group := range validGroups {
				if sub == group {
					sub = argv0
					break
				}
			}
		}

		// logger.Debug("match policy", zap.String("sub", sub), zap.String("obj", obj), zap.String("act", act), zap.String("file_dir", file_dir), zap.String("allow_deny", allow_deny))
		// 检查一下allow_deny, 如果与设置不符则报错
		if !((allow_deny == "allow" && policy_eff.args[0] == EFFECT_ALLOW) ||
			(allow_deny == "deny" && policy_eff.args[0] == EFFECT_DENY)) {
			logger.Fatal("policy_effect and allow_deny not match", zap.String("policy", match[0]))
		}

		/********** 获取map_idx(file or dir) **********/
		var map_idx int
		switch file_dir {
		case "file":
			map_idx = 1
		case "dir":
			map_idx = 2
		default:
			logger.Fatal("invalid policy file_dir", zap.String("policy", match[0]))
		}

		/********** 获取map_key **********/
		var map_key_s string // 这个实际上是path, 如果是dir, 则这个并不作为key
		// sub, obj都有的时候比较sub与要执行的程序名,
		// BUGS 与运行的程序*名称*一样则写入该规则(不完全匹配路径), 并将obj作为key
		if policy_def_map["sub"] && policy_def_map["obj"] {
			if path.Base(argv0) == path.Base(sub) {
				map_key_s = obj
			} else {
				continue
			}
		} else {
			// sub, obj只有一个则直接将其作为key
			map_key_s = match[1]
		}

		/********** 获取map_val并更新map **********/
		// 有act时, 需要根据查找结果判断是否需要更新
		// 没有act全部视为default, 目前是allow和deny全0
		op := -1 // 如果是is_rbac_act_args, 会直接存放多个arg的op bitmap, 其他情况就是单个op的数字
		if policy_def_map["act"] {
			if !is_rbac_act_args {
				// 获取并检查act
				op = checkOp(act)
				if op == -1 {
					logger.Fatal("invalid policy act", zap.String("policy", match[0]), zap.String("act", act))
				}
			}

			if is_args {
				if is_rbac_act_args {
					// 成组定义的act_args, 从之前解析的列表中获取解析结果即可
					argGroup, ok := actArgsMap[args]
					if !ok {
						logger.Fatal("args group not found", zap.String("groupName", args))
					}
					op = 0
					for _, arg := range argGroup {
						arg_key := HashUint32(HashUint32(uint32(arg.actOp)) + HashStr(map_key_s))
						arg_val := arg.argHash
						op |= 1 << arg.actOp
						// 更新arg_map
						// 并没有将判断arg_val是否为0放在解析group时, 因为这样在这里就可以报找不到group的错误
						if arg_val != 0 {
							logger.Info("update arg_map rabc", zap.Int("arg.actOp", arg.actOp), zap.String("map_key_s", map_key_s), zap.String("argGroup", args))
							err = ctx.maps[3].Update(arg_key, arg_val, ebpf.UpdateAny)
							if err != nil {
								logger.Fatal("update arg_map failed", zap.Error(err))
							}
						}
					}
				} else {
					// 一般的act_args, 需要在这里hash
					arg_key := HashUint32(HashUint32(uint32(op)) + HashStr(map_key_s))
					arg_val := hashArgs(args)
					if arg_val != 0 {
						logger.Info("update arg_map",
							zap.Int("op", op),
							zap.String("map_key_s", map_key_s),
							zap.String("args", args),
							zap.Uint32("arg_val", arg_val),
							zap.Uint32("bits", arg_val&7),
							zap.Uint32("arg_key", arg_key),
						)
						fmt.Println(ctx.maps)
						err = ctx.maps[3].Update(arg_key, arg_val, ebpf.UpdateAny)
						if err != nil {
							logger.Fatal("update arg_map failed", zap.Error(err))
						}
						var lookup_val_file uint32
						err = ctx.maps[3].Lookup(arg_key, &lookup_val_file)
						if err != nil {
							logger.Info("lookup arg_map failed", zap.Error(err))
						} else {
							logger.Info("lookup arg_map success", zap.Uint32("lookup_val_file", lookup_val_file))
						}
					}
				}
			}
		}
		// BUGS 这里有 if(matchers.args[0] == SUB_ACT)
		// 但是我这边不支持SUB_ACT, 所以暂时不写

		if file_dir == "file" {
			updateFileMap(ctx.maps[map_idx], map_key_s, op, allow_deny, is_rbac_act_args)
		} else {
			updateDirMap(ctx.maps[map_idx], map_key_s, op, allow_deny, dirCnt, is_rbac_act_args)
			dirCnt++
		}
	}

	// 更新DIR_ENTRY_IDX
	d_entry := C.line_t{}
	d_entry.nums = 1
	d_entry.args[0] = C.__u32(dirCnt)
	err = ctx.maps[0].Update(uint32(DIR_ENTRY_IDX), d_entry, ebpf.UpdateAny)
	if err != nil {
		logger.Fatal("update dirCnt failed", zap.Error(err))
	}
	logger.Debug("update DIR_ENTRY_IDX dirCnt", zap.Int("dirCnt", dirCnt))

	logger.Info("analyse policy success")
}

// 从acl_public.h中复制, 如果更改, 则需要同步更改下面的unmarshal函数
type policy_t struct {
	valid uint32
	allow uint32
	deny  uint32
}

const PATH_LEN int = 256

type dir_entry_t struct {
	path [PATH_LEN]byte
	rule policy_t
}

func unmarshal_polict_t(data []byte) (policy_t, error) {
	if len(data) != 12 {
		return policy_t{}, fmt.Errorf("invalid policy_t length")
	}
	return policy_t{
		valid: *(*uint32)(unsafe.Pointer(&data[0])),
		allow: *(*uint32)(unsafe.Pointer(&data[4])),
		deny:  *(*uint32)(unsafe.Pointer(&data[8])),
	}, nil
}

func unmarshal_dir_entry_t(data []byte) (dir_entry_t, error) {
	if len(data) != PATH_LEN+12 {
		return dir_entry_t{}, fmt.Errorf("invalid dir_entry_t length")
	}
	var path [PATH_LEN]byte
	copy(path[:], data[:PATH_LEN])
	rule, err := unmarshal_polict_t(data[PATH_LEN:])
	if err != nil {
		return dir_entry_t{}, fmt.Errorf("unmarshal dir_entry_t failed: %v", err)
	}
	return dir_entry_t{
		path: path,
		rule: rule,
	}, nil
}

// 解析获取map_val并update file_map
// op为-1说明没有act
func updateFileMap(bpfMap *ebpf.Map, map_key_s string, op int, allow_deny string, is_rbac_act_args bool) {
	logger := zap.L()

	var err error
	var map_val_file policy_t
	var lookup_val_file []byte
	map_key_b := make([]byte, bpfMap.KeySize())
	copy(map_key_b, map_key_s)

	// 有act(op)时, 需要根据查找结果判断是否需要更新
	// 没有act全部视为default, 目前是allow和deny全0
	if op != -1 {
		// 从file_map中查找
		err = bpfMap.Lookup(map_key_b, &lookup_val_file)
		if err != nil {
			if err.Error() != "lookup: key does not exist" {
				logger.Fatal("lookup policy failed", zap.String("key", map_key_s), zap.Error(err))
			}
		} else {
			// 查找成功, 已有对应规则, 则在原有规则上添加
			map_val_file, err = unmarshal_polict_t(lookup_val_file)
			if err != nil {
				logger.Fatal("unmarshal policy_t failed", zap.String("key", map_key_s), zap.Error(err))
			}
			logger.Debug("lookup policy success", zap.String("key", map_key_s),
				zap.Uint32("allow", map_val_file.allow),
				zap.Uint32("deny", map_val_file.deny),
			)
		}

		// 将规则写入bitmap
		// 查找失败时bitmap均为默认值, 故不需要额外处理
		if allow_deny == "allow" {
			if is_rbac_act_args {
				map_val_file.allow = map_val_file.allow | uint32(op)
			} else {
				map_val_file.allow = map_val_file.allow | (1 << op)
			}
		} else {
			if is_rbac_act_args {
				map_val_file.deny = map_val_file.deny | uint32(op)
			} else {
				map_val_file.deny = map_val_file.deny | (1 << op)
			}
		}
	}
	map_val_file.valid = 1

	// 更新file_map
	err = bpfMap.Update(map_key_b, map_val_file, ebpf.UpdateAny)
	if err != nil {
		logger.Fatal("update file policy failed", zap.String("key", map_key_s), zap.String("val", fmt.Sprintf("%v", map_val_file)), zap.Error(err))
	}
	logger.Debug("update file policy", zap.String("key", map_key_s), zap.String("val", fmt.Sprintf("%v", map_val_file)))
}

func updateDirMap(bpfMap *ebpf.Map, map_key_s string, op int, allow_deny string, dirCnt int, is_rbac_act_args bool) {
	logger := zap.L()

	var err error
	var map_val_dir dir_entry_t
	var map_key_i int // 这个是dir的index, 不命中则为dirCnt, 命中则为命中的index
	var dir_entry dir_entry_t
	var lookup_val_dir []byte

	// 获取dir_map的map_key
	// 因为dir_map的key只是作为id, 所以需要遍历整个dir_map
	// 不命中则为dirCnt, 命中则为命中的index
	for map_key_i = 0; map_key_i < dirCnt; map_key_i++ {
		err = bpfMap.Lookup(uint32(map_key_i), &lookup_val_dir)
		if err != nil {
			if err.Error() == "lookup: key does not exist" {
				// 理论上说, dirCnt以内的key都应该存在, 如果不存在则报错
				logger.Fatal("map_key_i unmatches dirCnt", zap.Int("map_key_i", map_key_i), zap.Int("dirCnt", dirCnt), zap.Error(err))
			} else {
				// 发生其他错误
				logger.Fatal("lookup policy failed", zap.Int("key", map_key_i), zap.Error(err))
			}
		} else {
			// 查找成功则需要比较内容是否相等
			dir_entry, err = unmarshal_dir_entry_t(lookup_val_dir)
			if err != nil {
				logger.Fatal("unmarshal dir_entry_t failed", zap.Int("map_key_i", map_key_i), zap.Error(err))
			}
			if map_key_s == string(dir_entry.path[:len(map_key_s)]) {
				// 找到已有的匹配项, 则直接使用该匹配项的index
				break
			}
		}
	}

	// 设置map_val_dir.path
	// dir由于获取map_key的时候已经遍历一遍, 这里不需要再次遍历
	if map_key_i == dirCnt {
		// 没有找到匹配项, 则需要新建一个
		copy(map_val_dir.path[:], map_key_s)
	} else {
		// 找到匹配项, 如果没有op, 则涉嫌重复定义, 报错
		if op == -1 {
			logger.Fatal("duplicate define dir", zap.String("key", map_key_s), zap.Int("map_key_i", map_key_i))
		}
		// 找到匹配项, 则直接使用原有结构体
		map_val_dir, err = unmarshal_dir_entry_t(lookup_val_dir)
		if err != nil {
			logger.Fatal("unmarshal dir_entry_t failed", zap.String("key", map_key_s), zap.Int("map_key_i", map_key_i), zap.Error(err))
		}
	}

	// 没有act全部视为default, 目前是allow和deny全0
	if op != -1 {
		// 如果op, 则将规则写入bitmap
		// 查找失败时bitmap均为默认值, 故不需要额外处理
		if allow_deny == "allow" {
			if is_rbac_act_args {
				map_val_dir.rule.allow = map_val_dir.rule.allow | uint32(op)
			} else {
				map_val_dir.rule.allow = map_val_dir.rule.allow | (1 << op)
			}
		} else {
			if is_rbac_act_args {
				map_val_dir.rule.deny = map_val_dir.rule.deny | uint32(op)
			} else {
				map_val_dir.rule.deny = map_val_dir.rule.deny | (1 << op)
			}
		}
	}
	map_val_dir.rule.valid = 1

	// 更新dir_map
	err = bpfMap.Update(uint32(map_key_i), map_val_dir, ebpf.UpdateAny)
	if err != nil {
		logger.Fatal("update dir policy failed", zap.String("key", map_key_s), zap.Int("map_key_i", map_key_i), zap.String("val", fmt.Sprintf("%v", map_val_dir)), zap.Error(err))
	}
	logger.Debug("update dir policy", zap.String("key", map_key_s), zap.Int("map_key_i", map_key_i), zap.String("val", fmt.Sprintf("%v", map_val_dir)))
}

func hashArgs(args string) uint32 {
	var hashRet uint32 = 0
	var hashVal uint32 = 0
	for i, arg := range strings.Split(args, ",") {
		if i >= 3 {
			break
		}
		if arg == "*" {
			continue
		}
		hashRet |= 1 << i
		if regexp.MustCompile(`[0-9]+`).MatchString(arg) {
			// 纯数字
			argNum, _ := strconv.Atoi(arg)
			hashVal += HashUint32(uint32(argNum))
		} else {
			// 字符串
			hashVal += HashStr(arg)
		}
	}
	if hashRet != 0 {
		hashRet |= HashUint32(hashVal) & 0xFFFFFFF8
	}
	return hashRet
}
