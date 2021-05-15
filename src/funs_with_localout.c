	#include"funs_with_localout.h"


	//�����ṩ�Ĳ������ô�ӡ������Ϣ����������ⲿdns��������ַ
	void ProcArgs(int argc, char* argv[])
	{
		for (int i = 1; i < argc; ++i)
		{
			if (argv[i][0] == '-')
			{
				if (argv[i][1] == 'd' && argv[i][2] == 'd')
					debugLevel = 2;
				else debugLevel = 1;
			}
			else
			{
				printf("set dns server:%s\n", argv[i]);
				strcpy(DNS_ADDRESS, argv[i]);
			}
		}

		printf("debug level %d\n", debugLevel);
	}


	//���ù���ʱ�䡣������Ҫ���õļ�¼ָ�������ʱ��   1
	void SetIdExpire(IDTransform* record, int ttl)
	{
		time_t now_time;
		now_time = time(NULL);
		record->expire_time = now_time + ttl;   //����ʱ��=����ʱ��+����ʱ��
	}

	//���record�Ƿ�ʱ
	int IsIdExpired(IDTransform* record)
	{
		time_t now_time;
		now_time = time(NULL);
		if (record->expire_time > 0 && now_time > record->expire_time)  //expire_time>0˵������Ч��¼
			return 1;
		return 0;
	}

	//������������IDת��Ϊ�µ�ID��������Ϣд��IDת������
	unsigned short RegisterNewID(unsigned short oID, SOCKADDR_IN temp, BOOL ifdone)
	{
		int i = 0;
		for (i = 0; i != AMOUNT; ++i)
		{
			//�ҵ��ѹ��ڻ�����������IDλ�ø���
			if (IsIdExpired(&IDTransTable[i]) == 1 || IDTransTable[i].done == TRUE)
			{
				IDTransTable[i].oldID = oID;    //������id
				IDTransTable[i].client = temp;  //������sockaddr
				IDTransTable[i].done = ifdone;  //�Ƿ����������
				SetIdExpire(&IDTransTable[i], ID_EXPIRE_TIME);
				++idNum;
				if (debugLevel >= 1)
					printf("%d id in id buffer\n", idNum);
				break;
			}
		}
		if (i == AMOUNT)    //û�ҵ���д�ĵط�
			return 0;
		return (unsigned short)i + 1;	//�Ա����±���Ϊ�µ�ID
	}

	//�ӱ���buf���ȡurl��dest���ʽ����3www5baidu3com0
	void ReadUrl(char* buf, char* dest)
	{
		int len = strlen(buf);
		int i = 0, j = 0, k = 0;
		while (i < len)
		{
			if (buf[i] > 0 && buf[i] <= 63) //����Ǹ�����
			{
				for (j = buf[i], i++; j > 0; j--, i++, k++) 
					//j�Ǽ����Ǽ���k��Ŀ��λ���±꣬i�Ǳ�������±�
					dest[k] = buf[i];
			}

			if (buf[i] != 0)    //���û��������dest��Ӹ�'.'
			{
				dest[k] = '.';
				k++;
			}
		}
		dest[k] = '\0';
	}

	void StandardPrint(char* buf, int length)   //-ddʱ�����ϸ��Ϣ
	{
		unsigned char temp;
		printf("receive len=%d: ", length);
		int i = 0;
		for (i = 0; i < length; i++)
		{
			temp = (unsigned char)buf[i];
			printf("%02x ", temp);
		}
		printf("\n");
	}

	//���url��ip�ں����������1�����򷵻�0
	int InBlack(char* query_url, const char* query_ip2)
	{
		int result;
		char* query_ip = NULL;
		*query_ip = *query_ip2;

		FILE* black_file;
		if ((black_file = fopen("blacklist.txt", "r")) == NULL) //�򿪺�������¼�ļ�
		{
			return 0;
		}

		char record[100];
		while (1)
		{
			if (fscanf(black_file, "%s", record) == EOF)    //������˽�β
			{
				result = 0;
				break;
			}
			else
			{
				if (strcmp(record, query_url) == 0 || strcmp(record, query_ip) == 0)    //����ֻ��������ip
				{
					result = 1;
					break;
				}
			}
		}

		fclose(black_file);
		return result;
	}


