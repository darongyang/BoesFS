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
//  line 2 : default_action : 见下俩
    //  在未命中时的逻辑，从黑白名单变成交由用户定义
    //  EFFECT_ALLOW : 允许
    //  EFFECT_DENY : 阻止
//  line 3 : matchers : 见下列enum matcher

//  model中的request_definition , policy_definition , policy_effect , matchers 存入model_map的下标
typedef enum model_map_idx{
    REQ_DEF_IDX,
    POL_DEF_IDX,
    POL_EFF_IDX,
    MATCHER_IDX,
    EXTRA_IDX,
    MAX_MODEL_IDX,
}model_map_idx_t;

//  matchers类型
typedef enum matcher{
    SUB_OBJ_ACT,        //  r.sub == p.sub && r.obj == p.obj && r.act == p.act
    SUB_OBJ,            //  r.sub == p.sub && r.obj == p.obj
    SUB_ACT,            //  r.sub == p.sub && r.act == p.act
    OBJ_ACT,            //  r.obj == p.obj && r.act == p.act
}matcher_t;

//  policy_effect类型
typedef enum default_action{
    EFFECT_ALLOW,       
    EFFECT_DENY         
}default_action_t;

/***************** Policy **************/
typedef struct policy{
    __u32 rule_map;            //  0 : deny ; 1 : allow
}policy_t;

//  acl_user.c use SETVALID to set bitmap 
#define SETVALID(x,y)	((x) | (1<<y)) 

//  acl_kern.c use ISVALID to check bitmap
#define ISVALID(n,x)	(((n) >> (x)) & 0x1)


#endif