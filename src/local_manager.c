#define _CRT_SECURE_NO_WARNINGS

#include "local_manager.h"
#include "funs_with_localout.h"



/*�����ļ��ṹ
int���и���

int������

struct
char[������][65]����url
int[������]url��Ӧip���ļ�λ�á����û�о���-1
int ����������ʱ����

MAX_RECORD��struct
char[16]ip
*/


//extern int debugLevel;

//���ù���ʱ�䡣������Ҫ���õļ�¼ָ�������ʱ��
void set_expire(int* expire_time, int ttl)
{
    //����0��ʾ��Ԥ�ȶ�cache.txt�趨��
    if (ttl == 0)
    {
        *expire_time = 0;
        return;
    }

    time_t now_time;
    now_time = time(NULL);
    *expire_time = now_time + ttl;
}

//���record�Ƿ�ʱ
int is_expired(int expire_time)
{
    //����0��ʾ��Ԥ���趨��
    if (expire_time == 0)
        return 0;

    time_t now_time;
    now_time = time(NULL);
    if (now_time > expire_time)
        return 1;
    return 0;
}

//����һ�������ļ�����ʼ��
void ini_cache_file(FILE* file)
{
    if (debugLevel >= 1)
        puts("ini cache file");

    int n_record = 0;   //�ļ������еļ�¼��
    int max_record = MAX_RECORD;
    stru_record_index empty_record_index;
    stru_record empty_record;

    //�»���������������-1��ʾΪ��
    memset(empty_record_index.url, 0, sizeof(empty_record_index.url));
    for (int i = 0; i < max_record; ++i)
        empty_record_index.index[i] = -1;

    fseek(file, 0, 0);
    fwrite(&n_record, sizeof(n_record), 1, file);   //�ļ������еļ�¼����0����
    fwrite(&max_record, sizeof(n_record), 1, file); //����¼��
    fwrite(&empty_record_index, sizeof(empty_record_index), 1, file);   //д�������
    for (int i = 0; i < max_record; ++i)
        fwrite(&empty_record, sizeof(empty_record), 1, file);   //д��max_record���ռ�¼
}

//�򻺴��ļ��������ַΪurl����ַΪaddr������ʱ��Ϊttl�ļ�¼
int add_record(char* url, char* addr, int ttl)
{
    FILE* cache_file;
    if ((cache_file = fopen("cache.dat", "r+")) == NULL)    //������治���ھͳ��Դ�������ʼ��
    {
        if (debugLevel >= 1)
            puts("Cache file no exist.");
        if ((cache_file = fopen("cache.dat", "w+")) == NULL)
        {
            puts("Can't create cache file.");
            return 1;
        }

        ini_cache_file(cache_file); //��ʼ��
        fseek(cache_file, 0, 0);    //�ļ�ָ�븴λ
    }

    //��ȡ����
    int n_record, max_record;
    fread((void*)&n_record, sizeof(n_record), 1, cache_file);
    fread((void*)&max_record, sizeof(max_record), 1, cache_file);

    //��ȡ����
    stru_record_index record_index;
    fread((void*)&record_index, sizeof(record_index), 1, cache_file);

    int url_pos = -1, empty_pos = -1;   //Ҫ������¼�¼д�뵽��λ�ã���һ������λ�á�����-1��ʾû�ҵ����õ�λ��  ��������
    int expired_pos = -1, first_expire_pos = -1;    //��һ����ʱ��λ�ã���û��ʱ�ļ�¼���һ����ʱ�ļ�¼��λ��  ����������

    //���ǲ����Ѿ������url�ļ�¼�ˡ�����о���֮ǰλ��д
    for (int i = 0; i < max_record; ++i)
    {
        int expire_time = record_index.expire_time[i];
        if (record_index.index[i] == -1)    //�����һ������λ�ã�������ܻ�Ѽ�¼д������    ����
        {
            if (empty_pos < 0)
                empty_pos = i;
        }
        else if (strcmp(url, record_index.url[i]) == 0)  //����ǿվͱȽ�url
        {
            url_pos = i;
            break;
        }
        else    //url����ͬ
        {
            if (expire_time > 0)
            {
                if (first_expire_pos < 0)
                    first_expire_pos = i;
                else if (expire_time < record_index.expire_time[first_expire_pos])  //�����糬ʱ�ļ�¼
                    first_expire_pos = i;
            }
            if (is_expired(expire_time) == 1) //�ǲ��ǳ�ʱ
            {
                expired_pos = i;
                break;
            }
        }
    }

    //���������û���url
    if (url_pos < 0)
    {
        if (expired_pos >= 0)   //�й��ڵľ͸��ǹ��ڵ�
        {
            url_pos = expired_pos;
            --n_record; //�м�¼Ҫ�����ǣ��Ȱ��������-1��������ټ�1
        }
        else if (empty_pos >= 0)    //û���ڵ�д����λ��
        {
            url_pos = empty_pos;
        }
        else if (first_expire_pos >= 0) //û���ڵ�û��λ�þ͸���������ڵ�
        {
            url_pos = first_expire_pos;
            --n_record; //�м�¼Ҫ�����ǣ��Ȱ��������-1��������ټ�1
        }
    }
    else --n_record;    //������

    if (url_pos >= 0) //�ҵ��˿�д�ĵط��Ÿ��»����ļ������򲻸Ļ����ļ�    ������
    {
        //��������
        strcpy(record_index.url[url_pos], url);
        record_index.index[url_pos] = url_pos;  //���Ի�����ipѡλ�õ��㷨
        set_expire(&record_index.expire_time[url_pos], ttl);
        fseek(cache_file, sizeof(int) + sizeof(int), 0);
        fwrite(&record_index, sizeof(record_index), 1, cache_file);

        //����ip��¼
        stru_record new_record;
        strcpy(new_record.addr, addr);
        fseek(cache_file, sizeof(int) + sizeof(int) + sizeof(stru_record_index) + url_pos * sizeof(stru_record), 0);
        fwrite(&new_record, sizeof(stru_record), 1, cache_file);

        //���¼���
        ++n_record;
        fseek(cache_file, 0, 0);
        fwrite(&n_record, sizeof(int), 1, cache_file);
    }

    fclose(cache_file);
    return 0;
}

//��ȡԤ�����õĻ���
void read_pre_cache()
{
    FILE* pre_cache_file;
    if ((pre_cache_file = fopen("cache.txt", "r")) == NULL)
        return;
    char url[65], ip[16];
    while (fscanf(pre_cache_file, "%s %s", url, ip) > 0)
    {
        if (debugLevel >= 1)
            printf("precache: add %s %s\n", url, ip);
        add_record(url, ip, 0);
    }
    fclose(pre_cache_file);
}

//ɾ��posλ�õļ�¼
void delete_record(FILE* file, int pos)
{
    if (debugLevel >= 1)
        puts("delete pos %d");

    //���¼���
    int n_r;
    fseek(file, 0, 0);
    fread(&n_r, sizeof(n_r), 1, file);
    --n_r;
    fseek(file, 0, 0);
    fwrite(&n_r, sizeof(n_r), 1, file);

    //��ȡ������
    stru_record_index record_index;
    fseek(file, sizeof(int) + sizeof(int), 0);
    fread(&record_index, sizeof(stru_record_index), 1, file);

    //��������
    record_index.url[pos][0] = '\0';
    record_index.index[pos] = -1;

    //д������
    fseek(file, sizeof(int) + sizeof(int), 0);
    fwrite(&record_index, sizeof(stru_record_index), 1, file);
}

//���صĵ�ַ��һ���ַ�Ϊ'n'��û���������'e'�������ˡ�'b'��������
ip_addr get_ip(char* query_url)
{
    ip_addr result;
    result.addr[0] = '\0';

    if (InBlack(query_url, "\0") == 1)
    {
        result.addr[0] = 'b';
        return result;
    }

    FILE* cache_file;
    if ((cache_file = fopen("cache.dat", "r+")) == NULL)
    {
        if (debugLevel >= 1)
            puts("Cache file no exist.");
        result.addr[0] = 'n';

        if ((cache_file = fopen("cache.dat", "w+")) != NULL)
        {
            ini_cache_file(cache_file); //��ʼ��
            fclose(cache_file);
        }
        else puts("Can't create cache file.");

        return result;
    }

    //��ȡ����
    int n_record, max_record;
    fread(&n_record, sizeof(n_record), 1, cache_file);
    fread(&max_record, sizeof(max_record), 1, cache_file);

    //��ȡ����
    stru_record_index record_index;
    fread(&record_index, sizeof(record_index), 1, cache_file);

    //������url��������λ��
    int url_index = -1;
    for (int i = 0; i < max_record; ++i)
    {
        if (record_index.index[i] != -1)
        {
            if (strcmp(query_url, record_index.url[i]) == 0)
            {
                //����ǲ��ǳ�ʱ
                if (is_expired(record_index.expire_time[i]) == 1)
                {
                    if (debugLevel >= 1)
                        puts("expired");
                    delete_record(cache_file, i);
                    result.addr[0] = 'e';
                }
                else url_index = i;
                break;
            }
        }
    }

    if (url_index == -1)   //������û�ҵ�url���Ҳ�������Ϊ��ʱ��û�ҵ�
    {
        if (result.addr[0] != 'e')
            result.addr[0] = 'n';
    }
    else
    {
        //����������ȡip��¼
        stru_record this_record;
        fseek(cache_file, sizeof(int) + sizeof(int) + sizeof(stru_record_index) + url_index * sizeof(stru_record), 0);
        fread(&this_record, sizeof(this_record), 1, cache_file);

        //���ip�ǲ����ں�����
        if (InBlack(query_url, this_record.addr) == 1)
            result.addr[0] = 'b';
        else strcpy(result.addr, this_record.addr);
    }

    fclose(cache_file);
    return result;
}

//��ʾ�����ļ�������
void disp()
{
    FILE* file;
    file = fopen("cache.dat", "r");
    if (file == NULL)
    {
        puts("disp::no exist.");
        return;
    }

    int a;
    fread(&a, sizeof(int), 1, file);
    printf("n_record:%d\n", a);
    fread(&a, sizeof(int), 1, file);
    printf("max_record:%d\n", a);

    stru_record_index record_index;
    stru_record record;

    fread(&record_index, sizeof(record_index), 1, file);
    for (int i = 0; i < a; ++i)
    {
        printf("%d:%d >%s<\n", i, record_index.index[i], record_index.url[i]);
    }

    for (int i = 0; i < a; ++i)
    {
        fread(&record, sizeof(record), 1, file);
        printf("%d:>%s< %d\n", i, record.addr, record_index.expire_time[i]);
    }

    fclose(file);
}