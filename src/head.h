#pragma once
#ifndef INC_HEAD_H
#define INC_HEAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <winsock2.h>   //put <winsock2.h> before <windows.h>
#include <windows.h> 

#pragma  comment(lib, "Ws2_32.lib") 

#define BUFSIZE 1024    //最大报文缓存大小
#define PORT_NO 53      //53端口号
#define AMOUNT 16       //最大ID转换表大小
#define ID_EXPIRE_TIME 10   //id过期时间
#define CACHE_EXPIRE 10     //高速缓存过期时间

typedef struct IDChange         //用于ID变换的结构体
{
    unsigned short oldID;		//原有ID
    BOOL done;					//标记是否完成解析
    SOCKADDR_IN client;			//请求者套接字地址
    int expire_time;            //ID过期时间
} IDTransform;

typedef unsigned short U16;
typedef struct _DNS_HDR     //DNS首部结构体
{
    U16 id;
    U16 tag;
    U16 numq;
    U16 numa;
    U16 numa1;
    U16 numa2;
}DNS_HDR;//DNS首部

extern int debugLevel;
extern char DNS_ADDRESS[16];       
extern IDTransform IDTransTable[AMOUNT];	//ID转换表
extern int idNum;					//转换表中的条目个数

extern WSADATA wsaData;//储存调用AfxSocketInit全局函数返回的Windows Sockets初始化信息
extern SOCKET localSock, outSock;//创建本地套接字，和外部套接字
extern int lenOfClient;  //len_client = sizeof client;
extern struct sockaddr_in localName, outName;//AF_INET地址
extern struct sockaddr_in localClient, outClient;

#endif