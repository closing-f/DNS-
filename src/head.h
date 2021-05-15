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

#define BUFSIZE 1024    //����Ļ����С
#define PORT_NO 53      //53�˿ں�
#define AMOUNT 16       //���IDת�����С
#define ID_EXPIRE_TIME 10   //id����ʱ��
#define CACHE_EXPIRE 10     //���ٻ������ʱ��

typedef struct IDChange         //����ID�任�Ľṹ��
{
    unsigned short oldID;		//ԭ��ID
    BOOL done;					//����Ƿ���ɽ���
    SOCKADDR_IN client;			//�������׽��ֵ�ַ
    int expire_time;            //ID����ʱ��
} IDTransform;

typedef unsigned short U16;
typedef struct _DNS_HDR     //DNS�ײ��ṹ��
{
    U16 id;
    U16 tag;
    U16 numq;
    U16 numa;
    U16 numa1;
    U16 numa2;
}DNS_HDR;//DNS�ײ�

extern int debugLevel;
extern char DNS_ADDRESS[16];       
extern IDTransform IDTransTable[AMOUNT];	//IDת����
extern int idNum;					//ת�����е���Ŀ����

extern WSADATA wsaData;//�������AfxSocketInitȫ�ֺ������ص�Windows Sockets��ʼ����Ϣ
extern SOCKET localSock, outSock;//���������׽��֣����ⲿ�׽���
extern int lenOfClient;  //len_client = sizeof client;
extern struct sockaddr_in localName, outName;//AF_INET��ַ
extern struct sockaddr_in localClient, outClient;

#endif