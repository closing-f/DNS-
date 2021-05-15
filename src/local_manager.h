//#pragma once
#ifndef INC_LOCAL_MANAGER_H_
#define INC_LOCAL_MANAGER_H_

#include <stdio.h>

#define MAX_RECORD 5

typedef struct
{
    char addr[16];
} ip_addr;

typedef struct
{
    char url[MAX_RECORD][65];   //url字符串
    int index[MAX_RECORD];  //对应url的ip在文件里的位置
    int expire_time[MAX_RECORD];    //过期时间
} stru_record_index;    //索引

typedef struct
{
    char addr[16];  //ip地址字符串
} stru_record;  //ip记录

//实际用的
//添加缓存，url是网址，addr是ip地址
int add_record(char* url, char* addr, int ttl);
//读取提前写好的cache.txt
void read_pre_cache();
//查询query_url的ip地址，返回结构体，结构体里的addr是ip地址字符串（形似123.234.222.111)
//如果结构体里的addr的第一个字符是'n'表示没缓存这个网址，'e'表示过期了，'b'网址或ip在黑名单里
//如果第一个字符不是以上几个字母就是查到的ip
ip_addr get_ip(char* query_url);


//debug用的
void ini_cache_file(FILE* file);    //创建缓存文件
void disp();    //打印缓存文件内容

#endif