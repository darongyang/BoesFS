#include <linux/fs.h>
#include "boesfs.h"
#include <linux/namei.h>

/*
 * BoesFS封装和实现了通过VFS的file对象、path对象、dentry对象获取绝对路径的方法
 */

/**
 * @brief Get the absolute path from path struct object
 *        从path结构（boesfs 和 bfs均可）中得到对应的绝对路径名
 * 
 * @param taget_path 目标的path结构（boesfs 和 bfs均可）
 * @param path_buf 一个可用的缓冲区，大小为PATH_MAX
 * @return char* 得到的绝对路径名
 */

char * get_absolute_path_from_path_struct(struct path * target_path, void *path_buf){

	if(path_buf == NULL){
		printk("Error: Failed with buf for file path\n");
		return NULL;
	}

	// path对象维护了挂载点路径和相对于挂载点的路径
	// 通过d_path直接获取绝对路径
    char * ab_path = d_path(target_path, (char *)path_buf, PATH_MAX);

	if (!ab_path) {
		printk("Error: Failed to get name from path!\n");
        return NULL;
	}

    return ab_path;
}


/**
 * @brief Get the absolute path from file struct object
 *        从file对象（boesfs 和 bfs均可）得到其对应的绝对路径名
 * 
 * @param taget_file 目标文件file对象（boesfs 和 bfs均可）
 * @param path_buf 一个可用的缓冲区，大小为PATH_MAX
 * @return char* 绝对路径名
 */
char * get_absolute_path_from_file_struct(struct file * target_file, void *path_buf){

	if(path_buf == NULL){
		printk("Error: Failed with buf for file path\n");
		return NULL;
	}

	// file对象对应一个path对象
	// 通过file_path直接获取绝对路径
	char * ab_path = file_path(target_file, (char *)path_buf, PATH_MAX);

	if (!ab_path) {
		printk("Error: Failed to get name from file!\n");
        return NULL;
	}

    return ab_path;
}

/**
 * @brief Get the absolute path from  dentry struct object
 *        从dentry对象（boesfs）得到其对应的绝对路径名
 * @param target_boesfs_dentry 目标dentry对象（只能是boesfs）
 * @return char* 绝对路径名
 */
char * get_absolute_path_from_dentry_struct(struct dentry * target_boesfs_dentry, void *path_buf){
	// 主要思路有两个
	// 1. boesfs dentry -> get lower path
	// 成功。但前提是boesfs dentry已经绑定过lower path了，什么时候绑定过呢?

	// 2. boesfs dentry -> dget_parent: boesfs parent_dentry -> get lower path: bfs parent_path -> 
	// 	  vfs_path_lookup: bfs low path -> get_absolute_path_from_path_struct
	// 失败。create为例，此时底层文件系统还没有这个文件，会lookup failed。

	// 利用了WrapFS的优势，会将wfs dentry和bfs path对象进行绑定

	char * ab_path;
	struct path lower_path;

	if(path_buf == NULL){
		printk("Error: Failed with buf for file path\n");
		return NULL;
	}

	// 获取底层path对象
	boesfs_get_lower_path(target_boesfs_dentry, &lower_path);

	// 借助path对象的方法
	ab_path = get_absolute_path_from_path_struct(&lower_path, path_buf);

	// 对应做释放，否则会一直引用
	boesfs_put_lower_path(target_boesfs_dentry, &lower_path);


	if (!ab_path) {
		printk("Error: Failed to get name from path!\n");
		return NULL;
	}

	return ab_path;
}
