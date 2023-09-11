#include "boesfs.h"


/* 
    解析Agent传来的指定格式的数据，得到指定字段值
    Agent数据格式："fd=%d,pid=%d,kill=%d,log=%d,logpath=%s,elfpath=%s"
*/

/**
 * @brief 解析来自Agent的数据
 * 
 * @param raw_data  来自Agent的数据
 * @param parse_data 接收解析结果的字符串数组
 * @return int 0:成功, -1:失败
 */
int boesfs_parse_agent_data(char* raw_data, char (*parse_int_data)[SINGLE_INT_MAX], char (*parse_path_data)[SINGLE_PATH_MAX]){
    char data_bak[RAW_DATA_MAX];
    int i, count=0;
    char separator = ','; // 分隔符
    int pos[DATA_TYPES_NUM+1]; // 标记分隔符","的位置

    // printk("raw_data: %s\n", raw_data);

    memcpy(data_bak, raw_data, strlen(raw_data) + 1);

    // printk("data_bak: %s\n", data_bak);

    // 获取分隔符","的位置  
    pos[0] = -1;
    count ++;
    for(i=0; i<strlen(data_bak); i++){
        if(data_bak[i]==separator && count < DATA_TYPES_NUM)
            pos[count++] = i;
    }
    pos[DATA_TYPES_NUM] = strlen(data_bak);

    // 解析
    for(i=0; i<INT_NUM; i++){
        memcpy(parse_int_data[i], &data_bak[pos[i]+1], pos[i+1]-pos[i]-1);
        parse_int_data[i][pos[i+1]-pos[i]-1] = '\0';
    }

    for(i=INT_NUM; i<DATA_TYPES_NUM; i++){
        memcpy(parse_path_data[i-INT_NUM], &data_bak[pos[i]+1], pos[i+1]-pos[i]-1);
        parse_path_data[i-INT_NUM][pos[i+1]-pos[i]-1] = '\0';
    }

    return 0;
}

/**
 * @brief 检查传入的参数是否符合预期格式
 * 
 * @param raw_data Agent mount时传来的数据
 * @param opt 解析选项
 * @return int -1: 不符合，>0:符合，并返回
 */
int boesfs_get_data_value(char * raw_data, int opt, void * value){
    int len;
    int i;
    char * pos; // fd=xxx，"xxx"的开始地址
    char * indentify;
    int ret;
    int is_int; // 所需参数是字符串还是整数

    if(value == NULL)
        return ERR;

    is_int = opt >= INT_NUM ? 0 : 1;

    char parse_int_data[INT_NUM][SINGLE_INT_MAX];
    char parse_path_data[PATH_NUM][SINGLE_PATH_MAX];

    ret = boesfs_parse_agent_data(raw_data, parse_int_data, parse_path_data);

    if(ret == -1){
        printk("ERROR: parse agent data ERROR!\n");
        return ERR;
    }
        
    switch(opt){
        case 0:
        indentify = "fd=";
        break;
        case 1:
        indentify = "pid=";
        break;
        case 2:
        indentify = "kill=";
        break;
        case 3:
        indentify = "log=";
        break;
        case 4:
        indentify = "logpath=";
        break;
        case 5:
        indentify = "elfpath=";
        break;
        default : 
        indentify = "fd=";
    }

    if(!is_int)
        opt = opt - INT_NUM;

    // 非法处理
    if(is_int){
        if(parse_int_data[opt]==NULL){
            printk("ERROR: parse_data is NULL!\n");
            return ERR;
        }

        if(!strstr(parse_int_data[opt],indentify)){
            printk("ERROR: parse_data is not valid!\n");
            return ERR;
        }
    }
    else{
        if(parse_path_data[opt]==NULL){
            printk("ERROR: parse_data is NULL!\n");
            return ERR;
        }

        if(!strstr(parse_path_data[opt],indentify)){
            printk("ERROR: parse_data is not valid!\n");
            return ERR;
        }
    }



    // 获取一份拷贝
    if(is_int){
        len = strlen(parse_int_data[opt]);
        char data[len+1];     
        memcpy(data, parse_int_data[opt], len+1);
        pos = &data[strlen(indentify)];

        // 判断合法性，大于0的数字
        if (!(strspn(pos, "0123456789") == strlen(pos))){
            printk("ERROR: value is not valid!\n");
            return ERR;
        }

        *(int *)value = simple_strtoul(pos, NULL, 10);
        // *(int *)value = atoi(pos);
        return 0; 
    }
    else{
        len = strlen(parse_path_data[opt]);
        char data[len+1];     
        memcpy(data, parse_path_data[opt], len+1);
        pos = &data[strlen(indentify)];

        memcpy((char *)value, pos, strlen(pos) + 1);
        return 0;
    }
}




/* Usage and Test Exsample */
// #include <stdio.h>
// #define INT_NUM 4
// #define PATH_NUM 2
// #define DATA_TYPES_NUM (INT_NUM + PATH_NUM) 
// #define SINGLE_INT_MAX 16
// #define SINGLE_PATH_MAX 256
// #define RAW_DATA_MAX (SINGLE_INT_MAX * INT_NUM + SINGLE_PATH_MAX * PATH_NUM)
// #define ERR -1
// int main(){
//     int value;
//     char path[SINGLE_PATH_MAX];
//     boesfs_get_data_value("fd=123,pid=1234,kill=1,log=0,logpath=/home/fs,elfpath=/home/fs/ls", 3, &value);
//     boesfs_get_data_value("fd=123,pid=1234,kill=1,log=1,logpath=/home/fs,elfpath=/home/fs/ls", 4, path);
//     printf("%d\n", value);
//     printf("%s\n", path);
// }

