// asrdemo_with_abnf.cpp : Defines the entry point for the console application.
//
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <Windows.h>

#include "../../include/qisr.h"


#ifdef _WIN64
#pragma comment(lib,"../../lib/msc_x64.lib")//x64
#else
#pragma comment(lib,"../../lib/msc.lib")//x86
#endif



const char* get_grammar( const char* filename );//��ȡ�﷨
void release_grammar(const char** grammar);//�ͷ��﷨ռ�õĿռ�
int run_asr(const char* grammar , const char* asrfile);//����ʶ��Ч��
const char*  get_audio_file(void);//ѡ����Ƶ�ļ�

const int BUFFER_NUM = 4096;
const int MAX_KEYWORD_LEN = 4096;

int _tmain(int argc, _TCHAR* argv[])
{
	const char* grammar = NULL;
	const char* asrfile = get_audio_file();
	int ret = MSP_SUCCESS;
	//appid ��������Ķ�
	ret = QISRInit("appid=52d8f781");
	if(ret != MSP_SUCCESS)
	{
		printf("QISRInit with errorCode: %d \n", ret);
		return 0;
	}

	grammar = get_grammar( "gm_continuous_digit.abnf" );
	if(ret != MSP_SUCCESS)
	{
		printf("getExID with errorCode: %d \n", ret);
		return 0;
	}

	ret = run_asr(grammar , asrfile);
	release_grammar(&grammar);
	
	QISRFini();
	char key = _getch();
	return 0;
}

const char* get_grammar( const char* filename )
{
	int ret = MSP_SUCCESS;
	int file_len = 0;
	char* grammar = NULL;
	FILE *fp=NULL; 
	fp=fopen(filename,"rb");
	if (NULL == fp)
	{
		printf("get_grammar| open file \"%s\" failed.\n",filename ? filename : "");
		return NULL;
	}
	fseek(fp,0L,SEEK_END);
	file_len=ftell(fp);
	fseek(fp,0L,SEEK_SET);

	grammar = (char*)malloc(file_len+1); //���ļ��ж�ȡ�﷨��ע���ı�����ΪGB2312
	fread((void*)grammar,1,file_len,fp);
	grammar[file_len]='\0'; //�ַ�����β
	fclose(fp);
	return grammar;
}

void release_grammar(const char** grammar)
{
	if (*grammar)
	{
		free((void*)*grammar);
		*grammar = NULL;
	}	
}

int run_asr(const char* grammar , const char* asrfile)
{
	int ret = MSP_SUCCESS;
	int i = 0;
	FILE* fp = NULL;
	char buff[BUFFER_NUM];
	unsigned int len;
	int status = MSP_AUDIO_SAMPLE_CONTINUE, ep_status = -1, rec_status = -1, rslt_status = -1;

	const char* param = "rst=plain,sub=asr,ssm=1,aue=speex,auf=audio/L16;rate=16000,grt=abnf";//ע��sub=asr,grt=abnf
	const char* sess_id = QISRSessionBegin(grammar, param, &ret);//���﷨����QISRSessionBegin
	if ( MSP_SUCCESS != ret )
	{
		printf("QISRSessionBegin err %d\n", ret);	
		return ret;
	}

	fp = fopen( asrfile , "rb");//�����ṩ�˼�����Ƶ�ļ�������ʱ������Ҫ���������
	if ( NULL == fp )
	{
		printf("failed to open file,please check the file.\n");
		QISRSessionEnd(sess_id, "normal");
		return -1;
	}

	printf("writing audio...\n");
	while ( !feof(fp) )
	{
		len = (unsigned int)fread(buff, 1, BUFFER_NUM, fp);
		feof(fp) ? status = MSP_AUDIO_SAMPLE_LAST : status = MSP_AUDIO_SAMPLE_CONTINUE;//���һ����ƵҪʹ��last
		ret = QISRAudioWrite(sess_id, buff, len, status, &ep_status, &rec_status);
		if ( ret != MSP_SUCCESS )
		{
			printf("\nQISRAudioWrite err %d\n", ret);
			break;
		}

		if ( rec_status == MSP_REC_STATUS_SUCCESS )
		{
			const char* result = QISRGetResult(sess_id, &rslt_status, 0, &ret);
			if (ret != MSP_SUCCESS )
			{
				printf("error code: %d\n", ret);
				break;
			}
			else if( rslt_status == MSP_REC_STATUS_NO_MATCH )
				printf("get result nomatch\n");
			else
			{
				if ( result != NULL )
					printf("get result[%d/%d]:len:%d\n %s\n", ret, rslt_status,strlen(result), result);
			}
		}
		printf(".");
		Sleep(200);//��Ϊ��ģ��¼����Ϊ�˱������ݷ���̫����ɻ��������������������ͣ200ms�������ʵʱ¼����������ͣ
	}
	printf("\n");

	if (ret == MSP_SUCCESS)
	{	
		printf("get reuslt\n");
		char asr_result[1024] = "";
		unsigned int pos_of_result = 0;
		int loop_count = 0;
		do 
		{
			const char* result = QISRGetResult(sess_id, &rslt_status, 0, &ret);
			if ( ret != 0 )
			{
				printf("QISRGetResult err %d\n", ret);
				break;
			}

			if( rslt_status == MSP_REC_STATUS_NO_MATCH )
			{
				printf("get result nomatch\n");
			}
			else if ( result != NULL )
			{
				printf("[%d]:get result[%d/%d]: %s\n", (loop_count), ret, rslt_status, result);
				strcpy(asr_result+pos_of_result,result);
				pos_of_result += (unsigned int)strlen(result);
			}
			else
			{
				printf("[%d]:get result[%d/%d]\n",(loop_count), ret, rslt_status);
			}
			Sleep(500);
		} while (rslt_status != MSP_REC_STATUS_COMPLETE && loop_count++ < 30);
		if (strcmp(asr_result,"")==0)
		{
			printf("no result\n");
		}

	}

	QISRSessionEnd(sess_id, NULL);
	printf("QISRSessionEnd.\n");
	fclose(fp); 

	return 0;
}

const char*  get_audio_file(void)
{
	char key = 0;
	while(key != 27)//��Esc���˳�
	{
		system("cls");//����
		printf("��ѡ����Ƶ�ļ���\n");
		printf("1.055165309093\n");
		printf("2.18012345678\n");
		printf("3.�ʱ�230088\n");
		printf("4.���տƴ�Ѷ����Ϣ�Ƽ��ɷ����޹�˾\n");
		printf("ע�⣺����������Ƶ�����ֺ����ֵĻ�ϣ����ֲ���Ҳ�ھ����ܵ������ֿ������Ŷȵ͡�\n      ������û�����֣�����Ϊ��������\n");
		key = _getch();
		switch(key)
		{
		case '1':
			printf("1.055165309093\n");
			return "wav/055165309093.wav";
		case '2':
			printf("2.18012345678\n");
			return "wav/18012345678.wav";
		case '3':
			printf("3.�ʱ�230088\n");
			return "wav/�ʱ�230088.wav";
		case '4':
			printf("4.���տƴ�Ѷ����Ϣ�Ƽ��ɷ����޹�˾\n");
			return "wav/���տƴ�Ѷ����Ϣ�Ƽ��ɷ����޹�˾.wav";
		default:
			continue;
		}
	}
	exit(0);
	return NULL;
}