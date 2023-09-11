//  acl_user.c 和 acl_kern.c 都需要的结构体 和 宏
//  假设model文件中，1种request_def，1种policy_def，1种poilcy_eff ，1种matcher
#ifndef _ACL_PUBLIC_H_
#define _ACL_PUBLIC_H_

#include<linux/types.h>

#define LINE_ARG_MAX 10

/******************  MODEL **************/
typedef struct line{
    __u32 nums;         //  参数个数
    __u32 args[LINE_ARG_MAX]; 
}line_t;
//  line 0 : request_definition :  args[0]:第0个request参数 ; arg[1]:第1个request参数 ...
//  line 1 : policy_definition : 同上
    //  虽然这俩没啥必要，不过还是传一下? 不传也行. 反正字节码目前没用到
//  line 2 : policy_effect : 见下俩enum policy_effect
    //  e = some(where (p.eft == deny))	X	casbin 不支持
    //  e = some(where (p.eft == allow ))	enum EFFECT_ALLOW
    //  e = !some(where (p.eft == deny))	enum EFFECT_DENY
    //  e = !some(where(p.eft == allow))	X	 casbin 不支持
//  line 3 : matchers : 见下列enum matcher

//  model中的request_definition , policy_definition , policy_effect , matchers 存入model_map的下标
typedef enum model_map_idx{
    REQ_DEF_IDX,
    POL_DEF_IDX,
    POL_EFF_IDX,
    MATCHER_IDX,
    DIR_ENTRY_IDX,          //  number of dir rules filled in dir map
    SUB_ACT_RULE_IDX,       //  only for matcher_sub_act. value : args[0] is allow bitmap ; args[1] is deny bitmap
    MAX_MODEL_IDX,
}model_map_idx_t;

//  matchers类型
typedef enum matcher{
    SUB_OBJ_ACT,        //  r.sub == p.sub && r.obj == p.obj && r.act == p.act
    SUB_OBJ,            //  r.sub == p.sub && r.obj == p.obj
    SUB_ACT,            //  r.sub == p.sub && r.act == p.act
    OBJ_ACT,            //  r.obj == p.obj && r.act == p.act
    SUB_OBJ_ACT_ARGS,   //  r.sub == p.sub && r.obj == p.obj && r.act == p.act && r.args == p.args
    // SUB_ACT_ARGS, 暂时不支持, 有ARGS一定要有OBJ和ACT
    OBJ_ACT_ARGS        //  r.obj == p.obj && r.act == p.act && r.args == p.args
}matcher_t;

//  policy_effect类型
typedef enum policy_effect{
    EFFECT_ALLOW,       //  e = some(where (p.eft == allow))
    EFFECT_DENY         //  e = !some(where (p.eft == deny))
}policy_effect_t;

/***************** Policy **************/
typedef struct policy{
    int valid;                 //  1 : 该节点存储的规则有效 ; 0 : 该节点存储的规则无效.  agent需要将这里置为1
    uint32_t allow;            //  allow bitmap
    uint32_t deny;             //  deny bitmap
}policy_t;

//  acl_user.c use SETVALID to set bitmap 
#define SETVALID(x,y)	((x) | (1<<y)) 

//  acl_kern.c use ISVALID to check bitmap
#define ISVALID(n,x)	(((n) >> (x)) & 0x1)


#define GETLOW3(x)      ((x) & 0x00000007)
#define GETHIGH29(x)    ((x) & 0xFFFFFFF8)


/**
 * @brief copy from acl_public.h
 */ 

/**
 * @brief BoesFS 哈希
 * 
 */

#define GETHIGH16(x) (((x) >> 16) & 0x0000FFFF)
#define GETLOW16(x) ((x) & 0x0000FFFF)
#define GETMULTIBASE(x) ((x+1) & 0x0000000F)
#define REVERSE16(x) ((GETLOW16(x) << 16) & 0xFFFF0000 | GETHIGH16(x))
#define HASHBASE1 428625750 // hash用于相加的基数
#define HASHBASE2 2004318071 // hash用于异或的基数
#define HASHBASE3 1431655765 // hash用于取模的基数
#define HASH(x) (REVERSE16(((REVERSE16(REVERSE16(REVERSE16(x)+(GETMULTIBASE(x)*HASHBASE1))) ^ HASHBASE2) % HASHBASE3)))

//  hash运算的类型
typedef enum hash_mode{
    OBJ_HASH, // 对客体名做hash
    OP_HASH, // 对操作做hash
    ARG1_HASH, // 对除客体名的第一个参数做hash
	ARG2_HASH, 
	ARG3_HASH, 
}hash_mode_t;

// 参数匹配判断
#define ARG1_VALID(x) ((x) & 0x00000001)
#define ARG2_VALID(x) ((x) & 0x00000002)
#define ARG3_VALID(x) ((x) & 0x00000004)

#endif