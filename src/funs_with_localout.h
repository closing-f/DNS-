//#pragma once
#ifndef INC_FUNS_WITH_LOCALOUT_H
#define INC_FUNS_WITH_LOCALOUT_H

#include"head.h"

void ProcArgs(int argc, char* argv[]);
//���ܣ������ṩ�Ĳ������ô�ӡ������Ϣ����������ⲿdns��������ַ
//������main()����Ĭ�ϲ�������������ʱ���͸�main�����������в����ĸ�����ָ����ַ���������ָ������

void SetIdExpire(IDTransform* record, int ttl);
//���ܣ����ù���ʱ��
//������Ҫ���õļ�¼ָ�������ʱ��

int IsIdExpired(IDTransform* record);
//���ܣ����record�Ƿ����
//������Ҫ���ļ�¼
//���أ����ڷ���0��δ���ڷ���1

unsigned short RegisterNewID(unsigned short oID, SOCKADDR_IN temp, BOOL ifdone);
//���ܣ�������IDת��Ϊ�µ�ID��������Ϣд��IDת������ 1
//�������ɵ�ID���׽��֣��Ƿ���������bool��
//���أ��µ�ID

void ReadUrl(char* buf, char* dest);
//���ܣ��ӱ���buf���ȡurl��dest�eg��3www5baidu3com0
//���������ģ��洢url���ַ�����

void StandardPrint(char* buf, int length);
//���ܣ�-ddʱ�����ϸ��Ϣ
//������DNS������Ϣ��DNS������Ϣ�ĳ���

int InBlack(char* query_url, const char* query_ip2);
//���ܣ�����Ƿ��ں������У�
//������query_url��Ҫ������ַ��query_ip2��Ҫ����ip��ַ
//���أ����url��ip�ں����������1�����򷵻�0

#endif