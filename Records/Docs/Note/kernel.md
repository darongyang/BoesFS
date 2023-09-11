# 内核函数和数据结构补充

[Linux源码查阅：Linux source code (v6.2.9) - Bootlin](https://elixir.bootlin.com/linux/latest/source)

## 内核函数

### **mount_nodev函数**

VFS用于挂载不需要设备(nodev)的文件系统的函数。

其直接为文件系统对象fs_type生成一个超级块对象s，并调用回调函数fill_super实现超级块对象s的填充。（而无需考虑设备相关操作）

其中data是mount_nodev提供的向回调函数fill_super传递的参数数据。

回调函数fill_super是文件系统根据自己的需求填充超级块s的自定义函数。

```c
struct dentry *mount_nodev(struct file_system_type *fs_type,
int flags, void *data,
int (*fill_super)(struct super_block *, void *, int))
{
int error;
struct super_block *s = sget(fs_type, NULL, set_anon_super, flags, NULL);

if (IS_ERR(s))
	return ERR_CAST(s);

error = fill_super(s, data, flags & SB_SILENT ? 1 : 0);
if (error) {
	deactivate_locked_super(s);
	return ERR_PTR(error);
}
s->s_flags |= SB_ACTIVE;
return dget(s->s_root);
}
EXPORT_SYMBOL(mount_nodev);
```

[参考资料： Kernel源码笔记之VFS：3.挂载_mount_nodev_苟浩的博客-CSDN博客](https://blog.csdn.net/jakelylll/article/details/123210796)

### **kmem_cache_xxx函数**

#### **①kmem_cache_create函数**

用于创建一个slab新缓存cache（kmem_cache结构体），在内核或者内核模块初始化的时候执行。

```c
struct kmem_cache * kmem_cache_create (const char *name, size_t size, size_t align, unsigned long flags, void (*ctor)(void *))
```

| 参数                 | 解释                                                                             |
| -------------------- | -------------------------------------------------------------------------------- |
| const char *name     | 缓存的标识名称                                                                   |
| size_t size          | 这个缓存的内存对象的大小，字节为单位                                             |
| size_t align         | 对齐方式                                                                         |
| unsigned long flags  | 分配缓存时的一些选项                                                             |
| void (*ctor)(void *) | 可选的对象构造器，回调函数，用户提供。用于该kmem_cache分配一个对象的初始化操作。 |

#### **②kmem_cache_destory函数**

用于销毁一个slab缓存cachep，在内核模块卸载的时候执行。注意在调用本函数的时候，缓存要为空。

```c
void kmem_cache_destroy(struct kmem_cache * cachep)
```

#### **③kmem_cache_alloc函数**

用于在一个slab缓存cachep（由create创建）中，申请分配一个内存对象并返回

```c
void *kmem_cache_alloc(struct kmem_cache *cachep, gfp_t f lags)
```

#### **④kmem_cache_zalloc函数**

和alloc一样，在alloc的基础上添加了初始化为0的操作

```c
static inline void *kmem_cache_zalloc(struct kmem_cache *k, gfp_t flags)
```

#### **⑤kmem_cache_free函数**

用于在一个slab缓存cachep中，释放一个申请过的内存对象objp，和alloc配套使用。

```c
void kmem_cache_free(struct kmem_cache *cachep, void *objp)
```

[参考资料：Linux内核API kmem_cache_create|极客笔记 (deepinout.com)](https://deepinout.com/linux-kernel-api/linux-kernel-api-memory-management/linux-kernel-api-kmem_cache_create.html)

[参考资料：Linux内核API kmem_cache_alloc|极客笔记 (deepinout.com)](https://deepinout.com/linux-kernel-api/linux-kernel-api-memory-management/linux-kernel-api-kmem_cache_alloc.html)

[参考资料：Linux内核API kmem_cache_destroy|极客笔记 (deepinout.com)](https://deepinout.com/linux-kernel-api/linux-kernel-api-memory-management/linux-kernel-api-kmem_cache_destroy.html)

[参考资料：Linux内核API kmem_cache_zalloc|极客笔记 (deepinout.com)](https://deepinout.com/linux-kernel-api/linux-kernel-api-memory-management/linux-kernel-api-kmem_cache_zalloc.html)

[参考资料：Linux内核API kmem_cache_free|极客笔记 (deepinout.com)](https://deepinout.com/linux-kernel-api/linux-kernel-api-memory-management/linux-kernel-api-kmem_cache_free.html)

### **register_filesystem函数**

VFS用于注册文件系统的一个函数

```c
int register_filesystem(struct file_system_type * fs)
```

### **d_make_root函数**

VFS用于为文件系统的根节点inode建立一个dentry，并返回。会先创建一个空的dentry，然后和root_inode绑定。

```c
struct dentry *d_make_root(struct inode *root_inode)
```

[参考资料： vfs dentry cache 模块实现分析/_大吉机器人的博客-CSDN博客](https://blog.csdn.net/uunubt/article/details/127278577)

### **container_of()函数/宏**

cotainer of是linux内核的一个宏定义：

```c
#define container_of(ptr, type, member) ({              \     
const typeof( ((type *)0)->member ) *__mptr = (ptr);    \     
(type *)( (char *)__mptr - offsetof(type,member) );})
```

其作用是：根据属于结构体type的一个成员member的起始地址ptr，得到结构体type的起始地址。

[参考资料： container of()函数简介_container_of_叨陪鲤的博客-CSDN博客](https://blog.csdn.net/s2603898260/article/details/79371024)

### **path_xxx()函数**

| 函数     | 作用                                                 |
| -------- | ---------------------------------------------------- |
| path_put | 给定一个path对象，将其对应的dentry对象的引用计数减一 |
| path_get | 给定一个path对象，将其对应的dentry对象的引用计数加一 |

[参考资料：path_put (kernel.org)](https://archive.kernel.org/oldlinux/htmldocs/filesystems/API-path-put.html)

[参考资料：path_get (kernel.org)](https://archive.kernel.org/oldlinux/htmldocs/filesystems/API-path-get.html)

### **dput()函数**

直接将dentry对象的引用数减一

[参考资料：Linux内核API dput|极客笔记 (deepinout.com)](https://deepinout.com/linux-kernel-api/linux-kernel-api-filesystem/linux-kernel-api-dput.html)

### **d_inode()函数**

获取一个dentry对象对应的inode对象

[参考资料：d_inode (kernel.org)](https://archive.kernel.org/oldlinux/htmldocs/filesystems/API-d-inode.html)

### **file_inode()函数**

获取一个file对象对应的inode对象

[fs.h - include/linux/fs.h - Linux source code (v6.2.9) - Bootlin](https://elixir.bootlin.com/linux/latest/source/include/linux/fs.h#L1353)

### **inode_init_once()函数**

对传入的inode做置空，和对每个成员都进行初始化操作

源码如下：

```c
void inode_init_once(struct inode *inode)
{
        memset(inode, 0, sizeof(*inode));
        __inode_init_once(inode); 
}
```

```c
void __inode_init_once(struct inode *inode)
{
        init_waitqueue_head(&inode->i_wait);
        INIT_LIST_HEAD(&inode->i_hash);
        INIT_LIST_HEAD(&inode->i_data.clean_pages);
        INIT_LIST_HEAD(&inode->i_data.dirty_pages);
        INIT_LIST_HEAD(&inode->i_data.locked_pages);
        INIT_LIST_HEAD(&inode->i_dentry);
        INIT_LIST_HEAD(&inode->i_dirty_buffers);
        INIT_LIST_HEAD(&inode->i_dirty_data_buffers);
        INIT_LIST_HEAD(&inode->i_devices);
        sema_init(&inode->i_sem, 1);
        sema_init(&inode->i_zombie, 1);
        init_rwsem(&inode->i_alloc_sem);
        spin_lock_init(&inode->i_data.i_shared_lock);
}
```

### **igrab()函数**

传入一个inode判断是否有效。①如果有效，则inode引用计数加一，同时将自身返回（grab抓住自身）。②如果无效，inode置空并返回。

内核源码如下：

```c
struct inode *igrab(struct inode *inode)
{
	spin_lock(&inode->i_lock);
	if (!(inode->i_state & (I_FREEING|I_WILL_FREE))) {
		__iget(inode);
		spin_unlock(&inode->i_lock);
	} else {
		spin_unlock(&inode->i_lock);
		/*
		 * Handle the case where s_op->clear_inode is not been
		 * called yet, and somebody is calling igrab
		 * while the inode is getting freed.
		 */
		inode = NULL;
	}
	return inode;
}
EXPORT_SYMBOL(igrab);
```

[参考资料：inode.c - fs/inode.c - Linux source code (v6.2.9) - Bootlin](https://elixir.bootlin.com/linux/latest/source/fs/inode.c#L1386)

### **iget5_locked()函数**

从一个已经挂载的文件系统获取一个指定的inode。

```c
struct inode * iget5_locked (	struct super_block * sb,
 	unsigned long hashval,
 	int (*test) (struct inode *, void *),
 	int (*set) (struct inode *, void *),
 	void * data);
```

①通过data和hashval得到散列值在inode cache查找inode，该散列桶中且满足test()函数，如果存在则增加inode引用并返回。

②如果不存在，则新建一个inode，上锁，设置flag为I_NEW，求hash值，并返回。（返回的上锁的inode可以被修改，修改完后再用unlock_new_inode()解锁即可。）

[参考资料：iget5_locked (kernel.org)](https://archive.kernel.org/oldlinux/htmldocs/filesystems/API-iget5-locked.html)

### **ilookup()函数**

在sb的inode cache中寻找对应ino的inode并返回。

```c
struct inode *ilookup(struct super_block *sb, unsigned long ino)
```

[参考资料：inode.c - fs/inode.c - Linux source code (v6.2.9) - Bootlin](https://elixir.bootlin.com/linux/latest/source/fs/inode.c#L1469)

### **truncate_inode_pages()函数**

从偏移lstart开始，将inode的mapping对应的内容清空

```c
void truncate_inode_pages(struct address_space *mapping, loff_t lstart)
```

[truncate.c - mm/truncate.c - Linux source code (v6.2.9) - Bootlin](https://elixir.bootlin.com/linux/latest/source/mm/truncate.c#L433)

### **clear_inode()函数**

将某个inode的标志位置为FREE和CLEAR，表示该inode释放了。

```c
void clear_inode(struct inode *inode)
{
    ...//断言检查
	inode->i_state = I_FREEING | I_CLEAR;
}
EXPORT_SYMBOL(clear_inode);
```

### **vfs_xxx()函数**

#### **vfs_statfs()函数**

根据传入的path对象，获取其对应的文件系统的统计信息，返回在buf中。

```c
int vfs_statfs(struct path * path, struct kstatfs *buf)
```

这些统计信息通常包括：文件系统的类型、块数、块的大小、文件数目、文件名长度等信息。

[Linux内核API vfs_statfs|极客笔记 (deepinout.com)](https://deepinout.com/linux-kernel-api/linux-kernel-api-filesystem/linux-kernel-api-vfs_statfs.html#:~:text=函数 vfs_statfs,() 根据第一个参数dentry获取整个文件系统的一些基本信息，将其保存在函数的第二个参数buf中，此基本信息包括当前文件系统的类型、文件系统的块数目、文件系统的块大小、文件系统的文件数目、文件系统的文件名字长度等信息。)

#### **vfs_read()函数**

vfs提供的读取指定文件内容到用户空间内存区域的接口函数。

原型如下：

```c
ssize_t vfs_read(struct file* filp, char __user* buffer, size_t len, loff_t* pos);
```

参数解释：

| 参数   | 解释                                                   |
| ------ | ------------------------------------------------------ |
| filp   | 待读取的文件对象                                       |
| buffer | 用户态指针，要读取到的地方。如果是内核态的指针会报错。 |
| len    | 读取数据长度                                           |
| pos    | 读取偏移指针，从哪里开始读                             |

#### **vfs_write()函数**

同vfs_read()。

```c
ssize_t vfs_write(struct file* filp, const char __user* buffer, size_t len, loff_t* pos);
```

[kernel中文件的读写操作可以使用vfs_read()和vfs_write - 韩国服务器-Time - 博客园 (cnblogs.com)](https://www.cnblogs.com/cbryge/p/6066978.html)

#### vfs_create()函数

用于创建一个新文件，将会创建一个inode，并和传入的dentry绑定。

```c
int vfs_create(struct user_namespace *mnt_userns, struct inode *dir,
	       struct dentry *dentry, umode_t mode, bool want_excl)
```

| 参数       | 解释                                |
| ---------- | ----------------------------------- |
| mnt_userns | 创建inode所在的用户空间？           |
| dir        | new file的父目录的inode             |
| dentry     | new file的空白dentry，还没绑定inode |
| mode       | new file的模式                      |
| want_excl  | new file是否一定要不存在            |

其他：

VFS目前导出给上层应用使用的函数接口（参Linux内核4.11.12源码，/include/linux/fs.h）主要包括：

```c
extern int vfs_create(struct inode *, struct dentry *, umode_t, bool);
extern int vfs_mkdir(struct inode *, struct dentry *, umode_t);
extern int vfs_mknod(struct inode *, struct dentry *, umode_t, dev_t);
extern int vfs_symlink(struct inode *, struct dentry *, const char *);
extern int vfs_link(struct dentry *, struct inode *, struct dentry *, struct inode **);
extern int vfs_rmdir(struct inode *, struct dentry *);
extern int vfs_unlink(struct inode *, struct dentry *, struct inode **);
extern int vfs_rename(struct inode *, struct dentry *, struct inode *, struct dentry *, struct inode **, unsigned int);
extern int vfs_whiteout(struct inode *, struct dentry *);

extern struct dentry *vfs_tmpfile(struct dentry *dentry, umode_t mode,
				  int open_flag);

extern ssize_t vfs_read(struct file *, char __user *, size_t, loff_t *);
extern ssize_t vfs_write(struct file *, const char __user *, size_t, loff_t *);
extern ssize_t vfs_readv(struct file *, const struct iovec __user *,
		unsigned long, loff_t *, int);
extern ssize_t vfs_writev(struct file *, const struct iovec __user *,
		unsigned long, loff_t *, int);
extern ssize_t vfs_copy_file_range(struct file *, loff_t , struct file *,
				   loff_t, size_t, unsigned int);
extern int vfs_clone_file_range(struct file *file_in, loff_t pos_in,
		struct file *file_out, loff_t pos_out, u64 len);

extern int vfs_statfs(const struct path *, struct kstatfs *);
extern int vfs_ustat(dev_t, struct kstatfs *);


extern long vfs_truncate(const struct path *, loff_t);
extern int vfs_fallocate(struct file *file, int mode, loff_t offset,
			loff_t len);

extern int vfs_fsync_range(struct file *file, loff_t start, loff_t end,
			   int datasync);
extern int vfs_fsync(struct file *file, int datasync);


extern loff_t vfs_llseek(struct file *file, loff_t offset, int whence);

ssize_t vfs_iter_read(struct file *file, struct iov_iter *iter, loff_t *ppos);
ssize_t vfs_iter_write(struct file *file, struct iov_iter *iter, loff_t *ppos);

extern int vfs_getattr(const struct path *, struct kstat *, u32, unsigned int);
extern int vfs_statx(int, const char __user *, int, struct kstat *, u32);
extern int vfs_statx_fd(unsigned int, struct kstat *, u32, unsigned int);
static inline int vfs_stat(const char __user *filename, struct kstat *stat);
static inline int vfs_lstat(const char __user *name, struct kstat *stat);
static inline int vfs_fstatat(int dfd, const char __user *filename,
			      struct kstat *stat, int flags);
static inline int vfs_fstat(int fd, struct kstat *stat);

extern const char *vfs_get_link(struct dentry *, struct delayed_call *);
extern int vfs_readlink(struct dentry *, char __user *, int);
```



### **fsstack_copy_attr_atime()函数**

拷贝后者inode的i_atime到前者inode的i_atime。

原型和定义如下：

```c
static inline void fsstack_copy_attr_atime(struct inode *dest,
					   const struct inode *src)
{
	dest->i_atime = src->i_atime;
}
```

### **d_splice_alias()函数**

将传入的inode和dentry关联起来。

```c
struct dentry *d_splice_alias(struct inode *inode, struct dentry *dentry)
```

https://elixir.bootlin.com/linux/latest/source/fs/dcache.c#L3136

### **vfs_path_lookup()函数**

根据filename，<parent_dentry，mnt>来在父目录parent_dentry下，查找filename对应的path对象，可以验证filename对应的dentry的有效性。

```c
int vfs_path_lookup(struct dentry *dentry, struct vfsmount *mnt,
		    const char *name, unsigned int flags,
		    struct path *path)
```



## **内核数据结构**

### **struct file_system_type对象**

在VFS中，每一个文件系统驱动程序都会对应一个文件系统对象struct file_system_type。

| 重要成员变量                 | 解释                                           |
| ---------------------------- | ---------------------------------------------- |
| const char *name             | 文件系统的名称，例如"wrapfs"                   |
| fs_flags                     | 文件系统的标识，例如FS_REQUIRES_DEV等          |
| struct module *owner         | 文件系统所属的内核模块，通常为THIS_MODULE      |
| struct dentry *(*mount)(...) | 文件系统挂载函数的指针，在mount -t ...时会调用 |
| void (*kill_sb)(...)         | 关闭文件系统实例时，释放超级块函数的指针       |

对于需要实现的mount()函数，通常调用mount_bdev、mount_nodev和mount_single函数实现，其参数解释如下：

```c
	struct dentry *(*mount) (struct file_system_type *, int,
		       const char *, void *); 
```

| 参数                              | 解释                                           |
| --------------------------------- | ---------------------------------------------- |
| struct file_system_type * fs_type | 要挂载的文件系统的对象指针                     |
| int flags                         | 挂载的标识                                     |
| const char * dev_name             | 挂载的设备名，例如在wrapfs中就是底层fs的路径名 |
| void* data                        | 挂载的选项                                     |

[参考资料：Linux VFS机制简析（一） - 舰队 - 博客园 (cnblogs.com)](https://www.cnblogs.com/jimbo17/p/10107318.html)

### **VFS对象**

#### **①struct super_block对象**

VFS四大对象之一，超级块。比较重要的成员有：

| 重要成员变量                                | 解释                                       |
| ------------------------------------------- | ------------------------------------------ |
| void             *s_fs_info                 | 关于超级块的一些额外信息                   |
| loff_t            s_maxbytes                | 允许的最大的文件大小(字节数)               |
| u32           s_time_gran                   | 文件系统支持的各种时间戳的最大可能的粒度。 |
| const struct super_operations    *s_op;     | 用户特定文件系统的操作超级块的函数集合     |
| const struct xattr_handler **s_xattr        | 拓展属性                                   |
| const struct export_operations *s_export_op | 导出方法，和NFS支持相关                    |
| struct dentry        *s_root                | 文件系统安装目录的目录项                   |
| atomic_t        s_active                    | 对超级块的引用计数                         |

[参考资料：VFS四大对象之一 struct super_block - yooooooo - 博客园 (cnblogs.com)](https://www.cnblogs.com/linhaostudy/p/7427027.html)

#### **②struct dentry对象**

#### **③struct inode对象**

#### **④struct file对象**

#### **⑤struct address_space对象（待补充）**

address_space对象用于：①管理page cache的page页。②关联某个文件的所有pages，并管理该文件的mmap。③提供内存管理接口、给定地址查page、查询page的tag等。

address_space充当了应用程序（如文件系统）和磁盘设备间的中间缓存管理。

磁盘设备->address_space：数据从磁盘中以page为单位读入address_space

address_space->应用程序：拷贝或者mapping方式提供

应用程序->address_space：先写入数据到address_space

address_space->磁盘设备：通过写回机制写回磁盘

### **VFS接口**

函数操作合集operations对象是VFS抽象出来提供管理对应对象的一组统一的接口，具体的实现交给不同的文件系统进行实现。常见的函数合集如下：

| 函数合集                 | 操作对象      |
| ------------------------ | ------------- |
| super_operations         | super         |
| dentry_operations        | dentry        |
| inode_operations         | inode         |
| file_operations          | file          |
| export_operations        | file handle   |
| vm_operations_struct     | vm            |
| address_space_operations | address_space |

#### **①struct super_operations对象**

在super block上有一系列的默认操作函数合集，就是struct super_operations对象。

| 重要成员变量                      | 解释                                                                                                                                                  |
| --------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------- |
| void (*put_super)(...)            | VFS释放超级块时调用，例如umount操作，前提是已经持有超级块的锁。                                                                                       |
| int (*statfs)(...)                | 用于获取文件系统的统计信息                                                                                                                            |
| int (*remount_fs)(...)            | 重新挂载文件系统，前提是持有kernel lock。                                                                                                             |
| void evict_inode(...)             | 用于彻底删除一个inode?                                                                                                                                |
| void (*umount_begin)(...)         | 用于unmount文件系统                                                                                                                                   |
| int (*show_options)(...)          | 用于/proc/mounts里显示文件系统挂载时的选项                                                                                                            |
| struct inode *(*alloc_inode)(...) | 分配一个inode并进行初始化                                                                                                                             |
| void (*destroy_inode)(...)        | 释放一个inode申请的资源                                                                                                                               |
| void (*drop_inode)(...)           | inode引用为0就调用该函数。该函数返回0，扔进LRU。该函数返回1，直接删除。如果文件系统无需inode缓存，可以直接设置为NULL或generic_delete_inode（返回1）。 |

[参考资料：Linux VFS机制简析（一） - 舰队 - 博客园 (cnblogs.com)](https://www.cnblogs.com/jimbo17/p/10107318.html)

#### **②struct dentry_operations对象**

dentry可以用来索引filename和inode number。在dentry上有一系列的操作函数合集，就是struct dentry_oparations对象。

文件系统可以对这组操作函数合集进行设置。定制好新的struct dentry_oparations结构体后，使用d_set_d_op函数即可对某个dentry进行d_op的设置。

| 重要成员变量             | 解释                             |
| ------------------------ | -------------------------------- |
| int (*d_revalidate)(...) | 用于检查dcache中的dentry是否有效 |
| void (*d_release)(...)   | 用于释放dentry                   |

[参考资料：Linux VFS机制简析（二） - 舰队 - 博客园 (cnblogs.com)](https://www.cnblogs.com/jimbo17/p/10119567.html)

#### **③struct inode_operations对象**

VFS用于管理inode对象的一系列函数操作的集合。

| 重要成员变量                   | 解释                                                         |
| ------------------------------ | ------------------------------------------------------------ |
| int (*readlink)(...)           |                                                              |
| int (*permission)(...)         |                                                              |
| int (*setattr)(...)            |                                                              |
| int (*getattr)(...)            |                                                              |
| (...)                          |                                                              |
| ssize_t (*listxattr)(...)      |                                                              |
| int (*create)(...)             | 目录下创建一个文件时会调用，完成inode的创建并和传入的dentry关联。只有目录类型的inode才有此方法 |
| struct dentry * (*lookup)(...) | 目录下查询一个inode时会调用，查找传入dentry的文件名name对应的inode并绑定到dentry，如果没找到就插入NULL。只有目录类型的inode才有此方法。 |
| int (*link)(...)               |                                                              |
| int (*unlink)(...)             |                                                              |
| int (*symlink)(...)            |                                                              |
| int (*mkdir)(...)              |                                                              |
| int (*rmdir)(...)              |                                                              |
| int (*mknod)(...)              |                                                              |
| int (*rename)(...)             |                                                              |
| int (*permission)(...)         |                                                              |
| int (*setattr)(...)            |                                                              |
| int (*getattr)(...)            |                                                              |

[参考资料：Linux VFS机制简析（一） - 舰队 - 博客园 (cnblogs.com)](https://www.cnblogs.com/jimbo17/p/10107318.html)

#### **④struct file_operations对象**

| 重要成员变量                | 解释                                                   |
| --------------------------- | ------------------------------------------------------ |
| loff_t (*llseek)(...)       |                                                        |
| ssize_t (*read)(...)        | 从设备中同步读取文件数据，读取成功返回读取的字节数。   |
| ssize_t (*write)(...)       | 从设备中写入文件数据。                                 |
| long (*unlocked_ioctl)(...) |                                                        |
| int （*mmap）(...)          |                                                        |
| int （*open）(...)          | VFS打开一个文件时会创建struct file和更新struct inode？ |
| int （*flush）(...)         | VFS关闭一个文件时会释放struct file和更新struct inode？ |
| int （*release）(...)       |                                                        |
| int （*fsync）(...)         |                                                        |
| int （*fasync）(...)        |                                                        |
| read_iter(...)              |                                                        |
| write_iter(...)             |                                                        |
| iterate(...)                |                                                        |

[参考资料：file_operations下函数详解(elecfans.com)](https://www.elecfans.com/emb/xitong/20110616202334.html)

#### **⑤struct export_operations对象**

在NFS中，引入了File Handle的概念。file handle默认包含ino和igen。有的文件系统可以自己定制file handle，为其添加新的内容。VFS提供一组操作file handle的接口struct export_operations，本地文件系统通过实现这组接口来完成file handle的生成和解析操作，从而支持NFS。

| 重要成员变量                         | 解释                                                                                                                                                                       |
| ------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| int (*encode_fh)(...)                | 打包ino和igen生成file handle，返回生成的file handle的类型。①提供了parent，连同parent打包，返回FILEID_INO32_GEN_PARENT。②没提供parent，只打包自己，返回FILEID_INO32_GEN。 |
| struct dentry * (*fh_to_dentry)(...) | 将给定fh_type类型的file handle解析出自己的dentry。                                                                                                                         |
| struct dentry * (*fh_to_parent)(...) | 将给定fh_type类型的file handle解析出parent的dentry。                                                                                                                       |

[Kernel File Handle 详解 - liuchao719 - 博客园 (cnblogs.com)](https://www.cnblogs.com/liuchao719/p/kernel-file-handle.html)

#### **⑥struct vm_operations_struct对象**

操作一段虚拟内存区域vm_area_struct的函数集合。

| 重要成员变量             | 解释                                                     |
| ------------------------ | -------------------------------------------------------- |
| void (*open)(...)        | 用于创建虚拟内存区域                                     |
| void (*close)(...)       | 用于删除虚拟内存区域                                     |
| int (*fault)(...)        | 当mmap发生缺页异常时，调用该处理函数读取文件数据到物理页 |
| int (*page_mkwrite)(...) | 将只能读的某个页变为可写的（通过cow）                    |

[参考资料：内存映射相关数据结构 _韩曙亮的博客-CSDN博客](https://blog.csdn.net/shulianghan/article/details/124105785)

#### **⑦struct address_space_operations对象**

| 重要成员变量              | 解释                                                 |
| ------------------------- | ---------------------------------------------------- |
| ssize_t (*direct_IO)(...) | 由通用读写流程调用，绕过page cache，直接和磁盘IO交互 |

[Linux VFS机制简析（二） - 舰队 - 博客园 (cnblogs.com)](https://www.cnblogs.com/jimbo17/p/10119567.html)

### **struct xattr_handler对象**

[Linux 文件系统扩展属性 xattr - xuyaowen - 博客园 (cnblogs.com)](https://www.cnblogs.com/xuyaowen/p/linux-xattrs.html)

[Linux VFS Extended Attribute And Access Control Table - 郑瀚Andrew - 博客园 (cnblogs.com)](https://www.cnblogs.com/LittleHann/p/4578675.html)

