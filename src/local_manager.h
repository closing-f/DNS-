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
    char url[MAX_RECORD][65];   //url�ַ���
    int index[MAX_RECORD];  //��Ӧurl��ip���ļ����λ��
    int expire_time[MAX_RECORD];    //����ʱ��
} stru_record_index;    //����

typedef struct
{
    char addr[16];  //ip��ַ�ַ���
} stru_record;  //ip��¼

//ʵ���õ�
//��ӻ��棬url����ַ��addr��ip��ַ
int add_record(char* url, char* addr, int ttl);
//��ȡ��ǰд�õ�cache.txt
void read_pre_cache();
//��ѯquery_url��ip��ַ�����ؽṹ�壬�ṹ�����addr��ip��ַ�ַ���������123.234.222.111)
//����ṹ�����addr�ĵ�һ���ַ���'n'��ʾû���������ַ��'e'��ʾ�����ˣ�'b'��ַ��ip�ں�������
//�����һ���ַ��������ϼ�����ĸ���ǲ鵽��ip
ip_addr get_ip(char* query_url);


//debug�õ�
void ini_cache_file(FILE* file);    //���������ļ�
void disp();    //��ӡ�����ļ�����

#endif