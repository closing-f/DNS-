#include"receive_from.h"
#include"local_manager.h"

//��Զ��DNS���ձ��Ĳ�ת��������
void RecvFromOut()
{
	char url[65], buffer[BUFSIZE];
	memset(buffer, 0, BUFSIZE);
	int length;
	length = -1;
	length = recvfrom(outSock, buffer, sizeof(buffer), 0, (struct sockaddr*)&outClient, &lenOfClient);//�����ⲿDNS������Ϣ

	if (length < 0)
	{
		if (debugLevel >= 1)
			printf("fail to receive\n", idNum);
	}
	else
	{
		if (debugLevel >= 2)
			standard_print(buffer, length);//��ӡ���ⲿ�ͻ��˽��յ���DNS������Ϣ

		 /*void *memcpy(void *destin, void *source, unsigned n)����Դsource�п���n���ֽڵ�Ŀ��destin�С�*/
		unsigned short *processID = (unsigned short *)malloc(sizeof(unsigned short));//�Խ���ID����ΪDNS���ĵ�һ�������ʾ��
		memcpy(processID, buffer, sizeof(unsigned short));//��Ϊ����ͷ���������ֽ�����ID��Ϣ
		int idIndex = (*processID) - 1;//IDת�����Ǵӵ�0�ʼ��
		free(processID);

		/*��IDת�����л�ȡ����DNS�����ߵ���Ϣ*/
		unsigned short clientID = IDTransTable[idIndex].oldID;//ת��Ϊ�ͻ��˷����ID
		memcpy(buffer, &clientID, sizeof(unsigned short));//���յ��ı��ĵ�ͷ���Ķ�Ӧ�ڷ���˵�ID���ö�Ӧ�ڿͻ��˵�ID���ǵ�
		IDTransTable[idIndex].done = TRUE;
		localClient = IDTransTable[idIndex].client;//�ӱ����ҵ�����DNS����Ŀͻ��˷�����
		--idNum;//�ɴ˽��Է��ʹ���DNS����Ŀͻ��˽��лظ���˵������������Ҫ�ظ���ϣ������������Ŀͻ��˾�����һ��
		if (debugLevel >= 1)
			printf("%d id in id bufferfer\n", idNum);
		int queryNum = ntohs(*((unsigned short*)(buffer + 4))), responseNum = ntohs(*((unsigned short*)(buffer + 6)));//����������ش����
		char* p = buffer + 12;//����DNS��ͷ��ָ��
		ip_addr ip;

		//��ȡÿ��������Ĳ�ѯurl
		for (int i = 0; i < queryNum; ++i)
		{
			readurl(p, url);    //��ôдurl��ֻ���¼���һ�������url����Ҫ�����������ԭ�򣬲����Լ�ȥ��

			while (*p > 0)  //��ȡ��ʶ��ǰ�ļ����������url
				p += (*p) + 1;
			p += 5; //����url�����Ϣ��ָ����һ������+5���а�����url������һ������0�Լ���ѯ���ͺͲ�ѯ�������ֶ�
		}

		if (responseNum > 0 && debugLevel >= 1)
			printf("receive outside %s\n", url);
		/*�����ظ�������ο�DNS�ظ����ĸ�ʽ*/
		for (int i = 0; i < responseNum; i++)
		{
			if ((unsigned char)*p == 0xc0) //��ָ�������
				p += 2;
			else
			{
				while (*p > 0)//��ȡ��ʶ��ǰ�ļ����������url
					p += (*p) + 1;
				p++;    //ָ����������
			}
			unsigned short responseType, responseClass, highBit, lowBit;
			int ttl, dataLen;
			responseType = ntohs(*(unsigned short*)p);//�ظ�����
			p += 2;
			responseClass = ntohs(*(unsigned short*)p);//�ظ���
			p += 2;
			highBit = ntohs(*(unsigned short*)p);//����ʱ���λ
			p += 2;
			lowBit = ntohs(*(unsigned short*)p);//����ʱ���λ
			p += 2;
			ttl = (((int)highBit) << 16) | lowBit;//�ߵ�λ��ϳ�����ʱ��
			dataLen = ntohs(*(unsigned short*)p);//���ݳ���
			p += 2;
			if (debugLevel >= 2)
				printf("type %d class %d ttl %d\n", responseType, responseClass, ttl);


			if (responseType != 1)//����A���ͣ�ֱ��������
			{
				p += dataLen;
			}
			else//��A���ͣ��ظ�����url��ip
			{
				int ip1, ip2, ip3, ip4;
				memset(ip.addr, 0, sizeof(ip.addr));
				//��ȡ4��ip����
				ip1 = (unsigned char)*p++;
				ip2 = (unsigned char)*p++;
				ip3 = (unsigned char)*p++;
				ip4 = (unsigned char)*p++;

				sprintf(ip.addr, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
				if (debugLevel >= 2)
					printf("ip %d.%d.%d.%d\n", ip1, ip2, ip3, ip4);

				/* ������ⲿ�������н��ܵ���������Ӧ��IP*/
				add_record(url, ip.addr, CACHE_EXPIRE);
				break;
			}
		}

		/*��bufferת���������ߴ���Ҳ���ǰѸ���Ӧ���ķ��͸��ͻ���*/
		length = sendto(localSock, buffer, length, 0, (SOCKADDR*)&localClient, sizeof(localClient));
		//printf("send local %s -> ip %d.%d.%d.%d\n", url, ip1, ip2, ip3, ip4);
	}
}

//�ӱ�����ȡDNS��ѯ���ӻ����ȡ���͵��ⲿDNS��������ѯ
void recvFromLocal()
{
	char url[65], buffer[BUFSIZE];
	memset(buffer, 0, BUFSIZE);

	int length;
	length = -1;
	length = recvfrom(localSock, buffer, sizeof buffer, 0, (struct sockaddr*)&localClient, &lenOfClient);//���ܱ���dns������
	if (length > 0)
	{
		char requestUrl[65];
		//printf("Recieve %d bytes\n",length);
		memcpy(requestUrl, &(buffer[sizeof(DNS_HDR)]), length);	//��ȡ�������е�������ʾ
		readurl(requestUrl, url);                     //��ȡ����������
		if (debugLevel >= 1)
			printf("local query %s\n", url);

		ip_addr ip = get_ip(url);          //�ӻ����в��Ҹ�������Ӧ��IP
		if (ip.addr[0] == 'b') //��ѯ��url��ip�ں�������
		{
			if (debugLevel >= 1)
				printf("%s in blacklist\n", url);
		}
		else if (ip.addr[0] == 'n' || ip.addr[0] == 'e')  //�ڻ�����δ�ҵ���Ӧ��IP���߸�������Ӧ��IP�Ѿ�����
		{
			//printf("%s not in cache or expired\n", url);
			unsigned short *processID = (unsigned short *)malloc(sizeof(unsigned short));
			memcpy(processID, buffer, sizeof(unsigned short));                 //��¼ID
			unsigned short severID = RegisterNewID(*processID, localClient, FALSE);   //����ID�͸÷��ͷ��ĵ�ַclient
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
				length = sendto(outSock, buffer, length, 0, (struct sockaddr*)&outName, sizeof(outName));  //���������͸��ⲿ������
			}
		}
		else
		{
			/*���ص�DNS������������Ӧ*/
			char sendBuffer[BUFSIZE];
			if (debugLevel >= 1)
				printf("cache read %s -> %s\n", url, ip.addr);

			memcpy(sendBuffer, buffer, length);						//����������
			unsigned short tag = htons(0x8180);
			memcpy(&sendBuffer[2], &tag, sizeof(unsigned short));		//�޸ı�־��

			unsigned short respNum;
			if (strcmp(ip.addr, "0.0.0.0") == 0) //�ж��Ƿ���Ҫ���θ������Ļش�
				respNum = htons(0x0000);	//���ι��ܣ����ش�����Ϊ0
			else  respNum = htons(0x0001);	//���������ܣ����ش�����Ϊ1
			memcpy(&sendBuffer[6], &respNum, sizeof(unsigned short));

			int currentLen = 0;
			char answer[16];
			unsigned short name = htons(0xc00c);//����ָ�루ƫ������
			memcpy(answer, &name, sizeof(unsigned short));
			currentLen += sizeof(unsigned short);

			unsigned short typeA = htons(0x0001);  //����
			memcpy(answer + currentLen, &typeA, sizeof(unsigned short));
			currentLen += sizeof(unsigned short);

			unsigned short classA = htons(0x0001);  //��ѯ��
			memcpy(answer + currentLen, &classA, sizeof(unsigned short));
			currentLen += sizeof(unsigned short);

			unsigned long servTime = htonl(0x7b);  //����ʱ��
			memcpy(answer + currentLen, &servTime, sizeof(unsigned long));
			currentLen += sizeof(unsigned long);

			unsigned short IPLen = htons(0x0004);  //��Դ���ݳ���
			memcpy(answer + currentLen, &IPLen, sizeof(unsigned short));
			currentLen += sizeof(unsigned short);

			unsigned long IP = (unsigned long)inet_addr(ip.addr);  //��Դ���ݼ�IP
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
				//��ȡ4��ip����
				ip1 = (unsigned char)*p++;
				ip2 = (unsigned char)*p++;
				ip3 = (unsigned char)*p++;
				ip4 = (unsigned char)*p++;
				printf("send local %s -> %d.%d.%d.%d\n", url, ip1, ip2, ip3, ip4);

			}

		}
	}
}
