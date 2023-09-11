#ifndef _DIR_TREE_H_
#define _DIR_TREE_H_

#include <linux/rbtree.h>
#include <linux/kfifo.h>

#define MAX_NODE_SIZE 64
#define FALSE 0
#define TRUE 1
#define PATH_MAX_SUPP 256

//  both for user and kernel
typedef struct policy{
    int valid;                 //  1 : 该节点存储的规则有效 ; 0 : 该节点存储的规则无效.         用户使用的policy_t定义不变.
    uint32_t allow;            //  allow bitmap
    uint32_t deny;             //  deny bitmap
}policy_t;

//  value of dir map
//  user , kernel 都会使用
typedef struct dir_entry{       
    char path[PATH_MAX_SUPP];
    policy_t rule;
}dir_entry_t; 


//  for kernel
typedef struct dir_node{
    char val[MAX_NODE_SIZE];         //  val后面需要置为'\0'
    int val_len;                     //  name len.
    policy_t rule;                   //  rule
    struct rb_root sub_root;         //  指向子树的根节点 root = RB_ROOT. dir_node 自身并不是子红黑树的根节点，只是有一个子红黑树根节点的指针（入口）; 其实就像定义成C++的map一样，这里是指向map的入口，供该节点进去查找，而非map中的红黑树本身的根节点。这个sub_root就指向子树的rb_node conn. sub root 和 子红黑树保持联系
    struct rb_node conn;             //  作为父节点的孩子, 与父目录保持连接, 使得父目录可以索引到该节点. conn 与父目录和同级的兄弟保持联系.  是否需要全部初始化为0 ???. 根节点的conn不起作用. 直接dir_tree_root指向根节点
}dir_node_t;


extern struct kmem_cache *dir_node_cache;                   //  slab内存池

struct dir_node *query_dir_tree(struct dir_node* dir_tree_root,const char *path);               //  查询
int insert_dir_tree(struct dir_node* dir_tree_root,const char *path , policy_t rule);
int destroy_dir_tree(struct dir_node* dir_tree_root);
// struct dir_node * build_dir_tree(void);
struct dir_node * init_dir_tree_root(void);

//  for bfs destory dir tree
#define BF_FIFO_ELEMENT_MAX 64

typedef struct bkfifo{
    struct kfifo queue;
}bkfifo_t;

//  定义fifo的元素 (使用结构体是为了便于增删内容). 保存根节点
typedef struct bkfifo_node {
    struct dir_node *dn;
}bkfifo_node_t;


#endif