#include"head.h"
#include"funs_with_localout.h"
#include"receive_from.h"
#include"local_manager.h"


int debugLevel = 0;                     //调试信息级别
char DNS_ADDRESS[16] = "192.168.1.107"; //DNS服务器，可修改
IDTransform IDTransTable[AMOUNT];	    //ID转换表
int idNum = 0;					        //ID转换表中的条目个数

WSADATA wsaData;    //储存调用AfxSocketInit全局函数返回的Windows Sockets初始化信息
SOCKET localSock, outSock;              //创建本地套接字，和外部套接字
int lenOfClient;    //len_client = sizeof client;
struct sockaddr_in localName, outName;  //AF_INET地址
struct sockaddr_in localClient, outClient;  


int main(int argc, char* argv[])
{
    ProcArgs(argc, argv);  //根据命令中的参数设置打印调试信息级别，以及设置外部dns服务器地址

    //初始化ID转换表
    int i = 0;
    for (i = 0; i < AMOUNT; i++)
    {
        IDTransTable[i].oldID = 0;
        IDTransTable[i].done = TRUE;
        IDTransTable[i].expire_time = 0;
        memset(&(IDTransTable[i].client), 0, sizeof(SOCKADDR_IN));
    }

    WSAStartup(MAKEWORD(2, 2), &wsaData);  //初始化Winsock服务，返回0为正常
    localSock = socket(AF_INET, SOCK_DGRAM, 0);//创建本地套接字
    outSock = socket(AF_INET, SOCK_DGRAM, 0);//创建外部套接字

    int unblock = 1;    //是否阻塞的标记
    ioctlsocket(outSock, FIONBIO, (u_long FAR*) & unblock);//将外部套接口设置为非阻塞
    ioctlsocket(localSock, FIONBIO, (u_long FAR*) & unblock);//将本地套接口设置为非阻塞
    if (localSock < 0)      //不成功
    {
        if (debugLevel >= 1)
            perror("create socket");
        exit(1);
    }

    outName.sin_family = AF_INET;           //Address family AF_INET代表TCP/IP协议族
    outName.sin_addr.s_addr = inet_addr(DNS_ADDRESS);        //外部DNS address
    outName.sin_port = htons(PORT_NO);        //Port number

    localName.sin_family = AF_INET;            //Address family 
    localName.sin_addr.s_addr = INADDR_ANY;          //本地任意 address
    localName.sin_port = htons(PORT_NO);            //Port number.DNS is 53

    int reuse = 1;
    setsockopt(localSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));//设置socket选项,重用，避免出现本地端口被占用的情况

    if (bind(localSock, (struct sockaddr*)&localName, sizeof(localName)) < 0)//绑定该套接字到53端口
    {
        if (debugLevel >= 1)
            perror("binding socket");
        exit(1);
    }
    lenOfClient = sizeof localClient;   

    read_pre_cache();//读取cache文件

    while (1)
    {
        receive_from_out();
        receive_from_local();
    }
}
