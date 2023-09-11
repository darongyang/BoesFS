#include "dir_tree.h"
#include "ebpf_helper.h"
#include <linux/slab.h>

struct kmem_cache *dir_node_cache = 1;           

//  /ab/cdef/hjk/lmn
//  返回最高的一级path name , 并返回比较的长度name_len
//  如 /ab/cdef/hjk/lmn , 返回 ab + len = 2
//  如 /lmn , 返回 lmn + len = 3
static const char *divide_name(const char *path,int *name_len)
{
    *name_len = 0;
    //  corner case : NULL. 
    if(*path == '\0'){
        *name_len = 0;
        return NULL;
    }

    //  corner case : " / "
    if(*path == '/' && *(path+1) == '\0'){
        *name_len = 1;
        return path;
    }
    
    //  normal case : " /ab/cdef/hjk/lmn "
    const char *start_idx = path+1;
    const char *cur_idx = path+1;
    while(*cur_idx != '\0') {
        if(*cur_idx != '/') {
           ++(*name_len);
           ++cur_idx; 
        } else {
            break;
        }
    }

    return start_idx;
}



//  寻找node的指向的红黑树的名为tmp_name的子节点, 没有则返回NULL. name_len : tmp_name长度. 不包含 '\0'. 
//  不会查询node节点本身
static struct dir_node* rb_find(struct dir_node *node,const char *tmp_name,int name_len)
{
    //  遍历node节点的 指向的 子红黑树
    //  never happened
    if(node == NULL) {
        printk(KERN_ERR "rb_find never happened node == null\n");
        return NULL;
    }
    
    struct rb_node *cur_node = node->sub_root.rb_node;       //  获取 map入口：node->children
    
    while (cur_node) {                                      //  struct Node *child = node->children[tmp_name];
        //  子节点 t
        struct dir_node *t = container_of(cur_node, struct dir_node, conn);
        int res = strncmp(tmp_name,t->val,max(name_len,t->val_len));
        if (res < 0) {
                cur_node = cur_node->rb_left;
        } else if (res > 0) {
            cur_node = cur_node->rb_right;
        } else {
            return t;                                   //  return child
        }
    }
    return NULL;
}



//  在字节码调用query , 返回path对应的节点
//  从dir_tree_root指向的dummy root作为根节点开始遍历
struct dir_node *query_dir_tree(struct dir_node* dir_tree_root,const char *path)
{
    //  corner case
    if(path == NULL) 
        return NULL;

    int tmp_len = 0;
    struct dir_node *cur = dir_tree_root;   //  指向dummy root
    struct dir_node *last = NULL;           //  指向要返回的节点
    const char *tmp_name = path;
    //  由于rb_find不会查询node节点本身，只会查询其下子节点
    //  故 root 第一个节点并没有被查询
    //  所以 root 不能指向有意义的节点，而是应该指向一个dummy root. dummy root之后再指向有意义的节点
    while(tmp_name && (*tmp_name)!='\0')                            //  假设n轮. 最后一轮从这里退出
    {
        tmp_name = divide_name(tmp_name,&tmp_len);          //  n轮加在一起 复杂度最坏 O(256)
        last = cur;                                         //  record the prev node
        cur = rb_find(cur,tmp_name,tmp_len);                //  n轮加在一起 复杂度最坏 O(n*(m*logm*256)).  假设每层m个节点 
        if(cur == NULL)
            break;
        tmp_name += tmp_len;
    }
    
    if(cur) {
        // printk("query dir found %s %d %d %d!\n",cur->val,cur->rule.valid,cur->rule.allow,cur->rule.deny);
        last = cur;
    } else {        
        if(last == dir_tree_root) {
            // printk("query dir not found %s! return root '/' as default , allow %d deny %d\n",path,last->rule.allow,last->rule.deny);
        } else {
            // printk("query dir not found %s! return parent %s as default\n",path,last->val);
        }
    }

    //  最终找到的最深父目录节点.
    return last;                                         
}


//  将data插入node下的子红黑树中. (data->conn 节点 插入到 node->sub_root.rb_bode)
//  0 代表 插入失败。在根节点应该为空的时候不为空
//  1 代表 插入成功。
//  可以处理node为dummy root，且无子节点的情况
    //  要插入new node
    //  最一开始只有dummy root. 那么所做的就是将指针指向new node. dummy root.sub_root.rb_node = new node
    //  dummy root不会参与红黑树的旋转，因为dummy root.sub_root 只是一个指向红黑树根节点的指针.
    //  红黑树节点的旋转不会影响dummy root. dummy root.sub_root保持指向子红黑树的根节点即可
//  可以处理node为有效node的情况，原理同上.
static int rb_insert(struct dir_node *node, struct dir_node *data , int tmp_len)
{
    struct rb_node **cur_node = &(node->sub_root.rb_node);
    struct rb_node *parent = NULL;

    /* Figure out where to put new node */
    while (*cur_node) {
            struct dir_node *t = container_of(*cur_node, struct dir_node, conn);
            int res = strncmp(data->val, t->val,max(tmp_len,t->val_len));
            parent = *cur_node;
            if (res < 0)
                cur_node = &((*cur_node)->rb_left);
            else if (res > 0)
                cur_node = &((*cur_node)->rb_right);
            else
                return FALSE;               //  如果已经存在，则返回FALSE. 插入失败
    }
    /* Add new node and rebalance tree. */
    //  插入data->conn节点并进行平衡
    rb_link_node(&data->conn, parent, cur_node);
    rb_insert_color(&data->conn,&(node->sub_root));     //  保持sub_root指向红黑树根节点.

    return TRUE;                            //  插入成功
}




//  在dir tree中插入新目录规则的node. 并再插入的过程中构造不存在的中间节点. 
    //  path即为node.val = /root/home/boes/a/b
    //  rule为node.rule. 注意rule.valid = 1
int insert_dir_tree(struct dir_node* dir_tree_root,const char *path , policy_t rule)
{

    struct dir_node *cur = dir_tree_root;
    const char *tmp_name = path;
    int tmp_len = 0;

    while(tmp_name && (*tmp_name)!='\0')
    {
        struct dir_node *parent = cur;
        tmp_name = divide_name(tmp_name,&tmp_len);
        if(*tmp_name == '/' && tmp_len == 1) {
            cur = dir_tree_root;
        } else {
            cur = rb_find(cur,tmp_name,tmp_len);
        }
        //  如果是我们要插入的目的节点. 也即tmp_name是path的的最后一段.
        int last = ((*(tmp_name + tmp_len)) == '\0');
        //  如果节点不存在
        if(cur == NULL)
        {
            //  这里是维护dir_tree的过程中，唯一会alloc节点的地方
            struct dir_node *node = (struct dir_node*) kmem_cache_alloc(dir_node_cache,GFP_KERNEL);     //  slab
            node->sub_root = RB_ROOT;                       //  init pointer to child rb_tree                          
            memcpy(node->val,tmp_name,tmp_len);             //  copy name
            node->val_len = tmp_len;                        //  记录name len. 比较的时候依赖于这个确定比较长度。 max(val_len,path_len)
            node->val[tmp_len] = '\0';                      //  注意结尾设为 '\0'. 比较的时候依赖于这个结束比较.
            if (last) {
                memcpy(&node->rule,&rule,sizeof(rule));     //  copy rule       //  user传入的rule.valid需要是1
                node->rule.valid = 1;                       //  rule.valid = 1. 告知字节码 这个节点里存储的规则是有效的，valid = 0则代表该目录并没有定义规则，应当采用默认行为。
            } else {
                memset(&node->rule,0,sizeof(node->rule));   //  否则置0
            }
            //  包含了 insert 根节点（parent为空）的情况
            if(rb_insert(parent,node,tmp_len)) {
                //  cur 指向node. 下一轮继续向下遍历
                cur = node;
            } else {
                // printk("%s node alreay exists. insert failed\n",tmp_name);
            }

        }
        //  如果节点存在 , 继续遍历即可.
        else 
        {
            //  如果已经遍历到我们要最终要插入的节点，那么记录规则即可
            if(last) {
                if(cur == dir_tree_root) {
                    printk("insert root node!\n");
                }
                cur->rule.allow |= rule.allow;               //  add rule
                cur->rule.deny |= rule.deny;                 //  add rule
                cur->rule.valid = 1;                        //  幂等行为. rule.valid = 1. 告知字节码 这个节点里存储的规则是有效的，valid = 0则代表该目录并没有定义规则，应当采用默认行为。
                // printk("target node %s exists allow %d deny %d\n",tmp_name,cur->rule.allow,cur->rule.deny);
            }
            //  如果不是最终要插入的，那么跳过，继续遍历
            else {
                // printk("path node %s exists , continue\n",tmp_name);
            }
        }
        tmp_name += tmp_len;
    }

    return TRUE;
}   

//  init dummy root. 
struct dir_node * init_dir_tree_root(void)
{
    struct dir_node * dir_tree_root =  (struct dir_node*) kmem_cache_alloc(dir_node_cache,GFP_KERNEL);
    dir_tree_root->sub_root = RB_ROOT;
    memcpy(dir_tree_root->val,"/",sizeof("/"));
    dir_tree_root->val_len = 1;
    dir_tree_root->rule.valid = 0;
    dir_tree_root->rule.allow = 0;
    dir_tree_root->rule.deny = 0;
    return dir_tree_root;
}

int destroy_dir_tree(struct dir_node* dir_tree_root)
{
    //  never happened
    if(dir_tree_root == NULL) {
        printk("destroy_dir_tree dir tree didn't init!\n");
        return FALSE;
    }
    //  动态申请队列
    struct bkfifo bf;
    int ret = kfifo_alloc(&bf.queue,sizeof(struct bkfifo_node)*BF_FIFO_ELEMENT_MAX,GFP_KERNEL);
    if(ret){
        return -1;
    }
    //  根节点入队
    struct bkfifo_node kn_root;
    kn_root.dn = dir_tree_root;
    kfifo_in(&bf.queue,&kn_root,sizeof(struct bkfifo_node));
    //  bfs 销毁dir_tree
    while(!kfifo_is_empty(&bf.queue))
    {
        //  取出kfifo.top()
        struct bkfifo_node top_bkn;
        kfifo_out(&bf.queue,&top_bkn,sizeof(struct bkfifo_node));
        struct rb_root *r = &(top_bkn.dn->sub_root);
        struct rb_node *node = NULL;
        //  在每一层将子节点全部存到一个队列中，然后删除该节点即可.
        for (node = rb_first(r); node; node = rb_next(node))
        {
            struct dir_node* t_dn = container_of(node,struct dir_node,conn);
            struct bkfifo_node t_bkn;
            t_bkn.dn = t_dn;
            kfifo_in(&bf.queue,&t_bkn,sizeof(struct bkfifo_node));    
        }
        //  释放dir_tree节点
        kmem_cache_free(dir_node_cache,top_bkn.dn);
    }
    //  释放kfifo
    kfifo_free(&bf.queue);

    return TRUE;
}
