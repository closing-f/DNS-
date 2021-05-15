	#include"funs_with_localout.h"


	//根据提供的参数设置打印调试信息级别和设置外部dns服务器地址
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


	//设置过期时间。参数是要设置的记录指针和生存时间   1
	void SetIdExpire(IDTransform* record, int ttl)
	{
		time_t now_time;
		now_time = time(NULL);
		record->expire_time = now_time + ttl;   //过期时间=现在时间+生存时间
	}

	//检查record是否超时
	int IsIdExpired(IDTransform* record)
	{
		time_t now_time;
		now_time = time(NULL);
		if (record->expire_time > 0 && now_time > record->expire_time)  //expire_time>0说明是有效记录
			return 1;
		return 0;
	}

	//函数：将请求ID转换为新的ID，并将信息写入ID转换表中
	unsigned short RegisterNewID(unsigned short oID, SOCKADDR_IN temp, BOOL ifdone)
	{
		int i = 0;
		for (i = 0; i != AMOUNT; ++i)
		{
			//找到已过期或已完成请求的ID位置覆盖
			if (IsIdExpired(&IDTransTable[i]) == 1 || IDTransTable[i].done == TRUE)
			{
				IDTransTable[i].oldID = oID;    //本来的id
				IDTransTable[i].client = temp;  //本来的sockaddr
				IDTransTable[i].done = ifdone;  //是否完成了请求
				SetIdExpire(&IDTransTable[i], ID_EXPIRE_TIME);
				++idNum;
				if (debugLevel >= 1)
					printf("%d id in id buffer\n", idNum);
				break;
			}
		}
		if (i == AMOUNT)    //没找到可写的地方
			return 0;
		return (unsigned short)i + 1;	//以表中下标作为新的ID
	}

	//从报文buf里读取url到dest里。格式类似3www5baidu3com0
	void ReadUrl(char* buf, char* dest)
	{
		int len = strlen(buf);
		int i = 0, j = 0, k = 0;
		while (i < len)
		{
			if (buf[i] > 0 && buf[i] <= 63) //如果是个计数
			{
				for (j = buf[i], i++; j > 0; j--, i++, k++) 
					//j是计数是几，k是目标位置下标，i是报文里的下标
					dest[k] = buf[i];
			}

			if (buf[i] != 0)    //如果没结束就在dest里加个'.'
			{
				dest[k] = '.';
				k++;
			}
		}
		dest[k] = '\0';
	}

	void StandardPrint(char* buf, int length)   //-dd时输出详细信息
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

	//如果url或ip在黑名单里，返回1，否则返回0
	int InBlack(char* query_url, const char* query_ip2)
	{
		int result;
		char* query_ip = NULL;
		*query_ip = *query_ip2;

		FILE* black_file;
		if ((black_file = fopen("blacklist.txt", "r")) == NULL) //打开黑名单记录文件
		{
			return 0;
		}

		char record[100];
		while (1)
		{
			if (fscanf(black_file, "%s", record) == EOF)    //出错或到了结尾
			{
				result = 0;
				break;
			}
			else
			{
				if (strcmp(record, query_url) == 0 || strcmp(record, query_ip) == 0)    //单行只有域名或ip
				{
					result = 1;
					break;
				}
			}
		}

		fclose(black_file);
		return result;
	}


