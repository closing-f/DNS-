//#pragma once
#ifndef INC_FUNS_WITH_LOCALOUT_H
#define INC_FUNS_WITH_LOCALOUT_H

#include"head.h"

void ProcArgs(int argc, char* argv[]);
//功能：根据提供的参数设置打印调试信息级别和设置外部dns服务器地址
//参数：main()函数默认参数，程序运行时发送给main函数的命令行参数的个数，指向的字符串参数的指针数组

void SetIdExpire(IDTransform* record, int ttl);
//功能：设置过期时间
//参数：要设置的记录指针和生存时间

int IsIdExpired(IDTransform* record);
//功能：检查record是否过期
//参数：要检查的记录
//返回：过期返回0，未过期返回1

unsigned short RegisterNewID(unsigned short oID, SOCKADDR_IN temp, BOOL ifdone);
//功能：将请求ID转换为新的ID，并将信息写入ID转换表中 1
//参数：旧的ID，套接字，是否完成请求的bool量
//返回：新的ID

void ReadUrl(char* buf, char* dest);
//功能：从报文buf里读取url到dest里。eg：3www5baidu3com0
//参数：报文，存储url的字符数组

void StandardPrint(char* buf, int length);
//功能：-dd时输出详细信息
//参数：DNS报文消息，DNS报文消息的长度

int InBlack(char* query_url, const char* query_ip2);
//功能：检查是否在黑名单中，
//参数：query_url：要检查的网址，query_ip2：要检查的ip地址
//返回：如果url或ip在黑名单里，返回1，否则返回0

#endif