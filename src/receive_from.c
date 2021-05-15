#include"receive_from.h"
#include"local_manager.h"

//从远端DNS接收报文并转发到本机
void RecvFromOut()
{
	char url[65], buffer[BUFSIZE];
	memset(buffer, 0, BUFSIZE);
	int length;
	length = -1;
	length = recvfrom(outSock, buffer, sizeof(buffer), 0, (struct sockaddr*)&outClient, &lenOfClient);//接受外部DNS报文消息

	if (length < 0)
	{
		if (debugLevel >= 1)
			printf("fail to receive\n", idNum);
	}
	else
	{
		if (debugLevel >= 2)
			standard_print(buffer, length);//打印从外部客户端接收到的DNS报文信息

		 /*void *memcpy(void *destin, void *source, unsigned n)；从源source中拷贝n个字节到目标destin中。*/
		unsigned short *processID = (unsigned short *)malloc(sizeof(unsigned short));//以进程ID来作为DNS报文的一个随机标示符
		memcpy(processID, buffer, sizeof(unsigned short));//因为报文头部的两个字节正是ID信息
		int idIndex = (*processID) - 1;//ID转换表是从第0项开始的
		free(processID);

		/*从ID转换表中获取发出DNS请求者的信息*/
		unsigned short clientID = IDTransTable[idIndex].oldID;//转换为客户端方向的ID
		memcpy(buffer, &clientID, sizeof(unsigned short));//对收到的报文的头部的对应于服务端的ID，用对应于客户端的ID覆盖掉
		IDTransTable[idIndex].done = TRUE;
		localClient = IDTransTable[idIndex].client;//从表中找到此条DNS请求的客户端发送者
		--idNum;//由此将对发送此条DNS请求的客户端进行回复，说明对他的请求要回复完毕，因此正在请求的客户端就少了一个
		if (debugLevel >= 1)
			printf("%d id in id bufferfer\n", idNum);
		int queryNum = ntohs(*((unsigned short*)(buffer + 4))), responseNum = ntohs(*((unsigned short*)(buffer + 6)));//问题个数；回答个数
		char* p = buffer + 12;//跳过DNS包头的指针
		ip_addr ip;

		//读取每个问题里的查询url
		for (int i = 0; i < queryNum; ++i)
		{
			readurl(p, url);    //这么写url里只会记录最后一个问题的url，主要是这个函数的原因，不信自己去看

			while (*p > 0)  //读取标识符前的计数跳过这个url
				p += (*p) + 1;
			p += 5; //跳过url后的信息，指向下一个报文+5其中包括，url中最后的一个计数0以及查询类型和查询类两个字段
		}

		if (responseNum > 0 && debugLevel >= 1)
			printf("receive outside %s\n", url);
		/*分析回复，具体参考DNS回复报文格式*/
		for (int i = 0; i < responseNum; i++)
		{
			if ((unsigned char)*p == 0xc0) //是指针就跳过
				p += 2;
			else
			{
				while (*p > 0)//读取标识符前的计数跳过这个url
					p += (*p) + 1;
				p++;    //指向后面的内容
			}
			unsigned short responseType, responseClass, highBit, lowBit;
			int ttl, dataLen;
			responseType = ntohs(*(unsigned short*)p);//回复类型
			p += 2;
			responseClass = ntohs(*(unsigned short*)p);//回复类
			p += 2;
			highBit = ntohs(*(unsigned short*)p);//生存时间高位
			p += 2;
			lowBit = ntohs(*(unsigned short*)p);//生存时间低位
			p += 2;
			ttl = (((int)highBit) << 16) | lowBit;//高低位组合成生存时间
			dataLen = ntohs(*(unsigned short*)p);//数据长度
			p += 2;
			if (debugLevel >= 2)
				printf("type %d class %d ttl %d\n", responseType, responseClass, ttl);


			if (responseType != 1)//不是A类型，直接跳过，
			{
				p += dataLen;
			}
			else//是A类型，回复的是url的ip
			{
				int ip1, ip2, ip3, ip4;
				memset(ip.addr, 0, sizeof(ip.addr));
				//读取4个ip部分
				ip1 = (unsigned char)*p++;
				ip2 = (unsigned char)*p++;
				ip3 = (unsigned char)*p++;
				ip4 = (unsigned char)*p++;

				sprintf(ip.addr, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
				if (debugLevel >= 2)
					printf("ip %d.%d.%d.%d\n", ip1, ip2, ip3, ip4);

				/* 缓存从外部服务器中接受到的域名对应的IP*/
				add_record(url, ip.addr, CACHE_EXPIRE);
				break;
			}
		}

		/*把buffer转发至请求者处，也就是把该响应报文发送给客户端*/
		length = sendto(localSock, buffer, length, 0, (SOCKADDR*)&localClient, sizeof(localClient));
		//printf("send local %s -> ip %d.%d.%d.%d\n", url, ip1, ip2, ip3, ip4);
	}
}

//从本机读取DNS查询，从缓存读取或发送到外部DNS服务器查询
void recvFromLocal()
{
	char url[65], buffer[BUFSIZE];
	memset(buffer, 0, BUFSIZE);

	int length;
	length = -1;
	length = recvfrom(localSock, buffer, sizeof buffer, 0, (struct sockaddr*)&localClient, &lenOfClient);//接受本地dns请求报文
	if (length > 0)
	{
		char requestUrl[65];
		//printf("Recieve %d bytes\n",length);
		memcpy(requestUrl, &(buffer[sizeof(DNS_HDR)]), length);	//获取请求报文中的域名表示
		readurl(requestUrl, url);                     //获取报文中域名
		if (debugLevel >= 1)
			printf("local query %s\n", url);

		ip_addr ip = get_ip(url);          //从缓存中查找该域名对应的IP
		if (ip.addr[0] == 'b') //查询的url或ip在黑名单里
		{
			if (debugLevel >= 1)
				printf("%s in blacklist\n", url);
		}
		else if (ip.addr[0] == 'n' || ip.addr[0] == 'e')  //在缓存中未找到对应的IP或者该域名对应的IP已经过期
		{
			//printf("%s not in cache or expired\n", url);
			unsigned short *processID = (unsigned short *)malloc(sizeof(unsigned short));
			memcpy(processID, buffer, sizeof(unsigned short));                 //记录ID
			unsigned short severID = RegisterNewID(*processID, localClient, FALSE);   //储存ID和该发送方的地址client
			free(processID);
			if (severID == 0)
			{
				if (debugLevel >= 1)
					puts("buffer full.");
			}
			else
			{
				if (debugLevel >= 1)
					printf("send outside %s\n", url);
				memcpy(buffer, &severID, sizeof(unsigned short));
				length = sendto(outSock, buffer, length, 0, (struct sockaddr*)&outName, sizeof(outName));  //将该请求发送给外部服务器
			}
		}
		else
		{
			/*本地的DNS服务器进行响应*/
			char sendBuffer[BUFSIZE];
			if (debugLevel >= 1)
				printf("cache read %s -> %s\n", url, ip.addr);

			memcpy(sendBuffer, buffer, length);						//拷贝请求报文
			unsigned short tag = htons(0x8180);
			memcpy(&sendBuffer[2], &tag, sizeof(unsigned short));		//修改标志域

			unsigned short respNum;
			if (strcmp(ip.addr, "0.0.0.0") == 0) //判断是否需要屏蔽该域名的回答
				respNum = htons(0x0000);	//屏蔽功能：将回答数置为0
			else  respNum = htons(0x0001);	//服务器功能：将回答数置为1
			memcpy(&sendBuffer[6], &respNum, sizeof(unsigned short));

			int currentLen = 0;
			char answer[16];
			unsigned short name = htons(0xc00c);//域名指针（偏移量）
			memcpy(answer, &name, sizeof(unsigned short));
			currentLen += sizeof(unsigned short);

			unsigned short typeA = htons(0x0001);  //类型
			memcpy(answer + currentLen, &typeA, sizeof(unsigned short));
			currentLen += sizeof(unsigned short);

			unsigned short classA = htons(0x0001);  //查询类
			memcpy(answer + currentLen, &classA, sizeof(unsigned short));
			currentLen += sizeof(unsigned short);

			unsigned long servTime = htonl(0x7b);  //生存时间
			memcpy(answer + currentLen, &servTime, sizeof(unsigned long));
			currentLen += sizeof(unsigned long);

			unsigned short IPLen = htons(0x0004);  //资源数据长度
			memcpy(answer + currentLen, &IPLen, sizeof(unsigned short));
			currentLen += sizeof(unsigned short);

			unsigned long IP = (unsigned long)inet_addr(ip.addr);  //资源数据即IP
			memcpy(answer + currentLen, &IP, sizeof(unsigned long));
			currentLen += sizeof(unsigned long);
			currentLen += length;
			memcpy(sendBuffer + length, answer, sizeof(answer));
			length = sendto(localSock, sendBuffer, currentLen, 0, (SOCKADDR*)&localClient, sizeof(localClient));

			if (length < 0)
				perror("recv outside len < 0");

			char *p;
			p = sendBuffer + length - 4;
			if (debugLevel >= 1)
			{
				int ip1, ip2, ip3, ip4;
				memset(ip.addr, 0, sizeof(ip.addr));
				//读取4个ip部分
				ip1 = (unsigned char)*p++;
				ip2 = (unsigned char)*p++;
				ip3 = (unsigned char)*p++;
				ip4 = (unsigned char)*p++;
				printf("send local %s -> %d.%d.%d.%d\n", url, ip1, ip2, ip3, ip4);

			}

		}
	}
}
