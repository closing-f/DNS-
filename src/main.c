#include"head.h"
#include"funs_with_localout.h"
#include"receive_from.h"
#include"local_manager.h"


int debugLevel = 0;                     //������Ϣ����
char DNS_ADDRESS[16] = "192.168.1.107"; //DNS�����������޸�
IDTransform IDTransTable[AMOUNT];	    //IDת����
int idNum = 0;					        //IDת�����е���Ŀ����

WSADATA wsaData;    //�������AfxSocketInitȫ�ֺ������ص�Windows Sockets��ʼ����Ϣ
SOCKET localSock, outSock;              //���������׽��֣����ⲿ�׽���
int lenOfClient;    //len_client = sizeof client;
struct sockaddr_in localName, outName;  //AF_INET��ַ
struct sockaddr_in localClient, outClient;  


int main(int argc, char* argv[])
{
    ProcArgs(argc, argv);  //���������еĲ������ô�ӡ������Ϣ�����Լ������ⲿdns��������ַ

    //��ʼ��IDת����
    int i = 0;
    for (i = 0; i < AMOUNT; i++)
    {
        IDTransTable[i].oldID = 0;
        IDTransTable[i].done = TRUE;
        IDTransTable[i].expire_time = 0;
        memset(&(IDTransTable[i].client), 0, sizeof(SOCKADDR_IN));
    }

    WSAStartup(MAKEWORD(2, 2), &wsaData);  //��ʼ��Winsock���񣬷���0Ϊ����
    localSock = socket(AF_INET, SOCK_DGRAM, 0);//���������׽���
    outSock = socket(AF_INET, SOCK_DGRAM, 0);//�����ⲿ�׽���

    int unblock = 1;    //�Ƿ������ı��
    ioctlsocket(outSock, FIONBIO, (u_long FAR*) & unblock);//���ⲿ�׽ӿ�����Ϊ������
    ioctlsocket(localSock, FIONBIO, (u_long FAR*) & unblock);//�������׽ӿ�����Ϊ������
    if (localSock < 0)      //���ɹ�
    {
        if (debugLevel >= 1)
            perror("create socket");
        exit(1);
    }

    outName.sin_family = AF_INET;           //Address family AF_INET����TCP/IPЭ����
    outName.sin_addr.s_addr = inet_addr(DNS_ADDRESS);        //�ⲿDNS address
    outName.sin_port = htons(PORT_NO);        //Port number

    localName.sin_family = AF_INET;            //Address family 
    localName.sin_addr.s_addr = INADDR_ANY;          //�������� address
    localName.sin_port = htons(PORT_NO);            //Port number.DNS is 53

    int reuse = 1;
    setsockopt(localSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));//����socketѡ��,���ã�������ֱ��ض˿ڱ�ռ�õ����

    if (bind(localSock, (struct sockaddr*)&localName, sizeof(localName)) < 0)//�󶨸��׽��ֵ�53�˿�
    {
        if (debugLevel >= 1)
            perror("binding socket");
        exit(1);
    }
    lenOfClient = sizeof localClient;   

    read_pre_cache();//��ȡcache�ļ�

    while (1)
    {
        receive_from_out();
        receive_from_local();
    }
}
