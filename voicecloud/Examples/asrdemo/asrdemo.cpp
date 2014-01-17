// asrdemo.cpp : Defines the entry point for the console application.
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



int get_grammar_id( int upload );//��ȡ�﷨ID
int run_asr(const char* asrfile);//����ʶ��Ч��
const char*  get_audio_file(void);//ѡ����Ƶ�ļ�

char GrammarID[128];
const int BUFFER_NUM = 4096;
const int MAX_KEYWORD_LEN = 4096;

int _tmain(int argc, _TCHAR* argv[])
{
	int ret = MSP_SUCCESS;
	const char* asrfile = get_audio_file();
	//appid ��������Ķ�
	ret = QISRInit("appid=52d8f781");
	if(ret != MSP_SUCCESS)
	{
		printf("QISRInit with errorCode: %d \n", ret);
		return 0;
	}

	memset(GrammarID, 0, sizeof(GrammarID));
	ret = get_grammar_id( FALSE );
	if(ret != MSP_SUCCESS)
	{
		printf("get_grammar_id with errorCode: %d \n", ret);
		return 0;
	}

	ret = run_asr(asrfile);
	QISRFini();
	char key = _getch();
	return 0;
}

int get_grammar_id( int upload)
{
	int ret = MSP_SUCCESS;
	const char * sessionID = NULL;
	if (FALSE == upload)
	{
		strcpy(GrammarID, "e7eb1a443ee143d5e7ac52cb794810fe");
		//���ID�����ϴ�֮���¼�����ġ��﷨�ϴ�֮�����ñ����ڷ������ϣ����Բ�Ҫ�����ϴ�ͬ�����﷨
		return 0;
	}
	
	//�����Ҫ�����ϴ��﷨���������upload��ΪTRUE���Ϳ�����������ϴ��﷨����
	sessionID = QISRSessionBegin(NULL, "ssm=1,sub=asr", &ret);
	if(ret != MSP_SUCCESS)
	{
		printf("QISRSessionBegin with errorCode: %d \n", ret);
		return ret;
	}

	char UserData[MAX_KEYWORD_LEN];
	memset(UserData, 0, MAX_KEYWORD_LEN);
	FILE* fp = fopen("asr_keywords_utf8.txt", "rb");//�ؼ����б��ļ�������utf8��ʽ
	if (fp == NULL)
	{
		printf("keyword file cannot open\n");
		return -1;
	}
	unsigned int len = (unsigned int)fread(UserData, 1, MAX_KEYWORD_LEN, fp);
	UserData[len] = 0;
	fclose(fp);
	const char* testID = QISRUploadData(sessionID, "contact", UserData, len, "dtt=keylist", &ret);
	if(ret != MSP_SUCCESS)
	{
		printf("QISRUploadData with errorCode: %d \n", ret);
		return ret;
	}
	memcpy((void*)GrammarID, testID, strlen(testID));
	printf("GrammarID: \"%s\" \n", GrammarID);//����õ�GrammarID�������Ļ��

	QISRSessionEnd(sessionID, "normal");
	return 0;
}

int run_asr(const char* asrfile)
{
	int ret = MSP_SUCCESS;
	int i = 0;
	FILE* fp = NULL;
	char buff[BUFFER_NUM];
	unsigned int len;
	int status = MSP_AUDIO_SAMPLE_CONTINUE, ep_status = -1, rec_status = -1, rslt_status = -1;

	const char* param = "rst=plain,sub=asr,ssm=1,aue=speex,auf=audio/L16;rate=16000";//ע��sub=asr
	const char* sess_id = QISRSessionBegin(GrammarID, param, &ret);//���﷨ID����QISRSessionBegin
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
		printf("1.�ƴ�Ѷ��\n");
		printf("2.����ɽ����\n");
		printf("3.��³ʯ��\n");
		printf("4.һ�����������߰˾�ʮ\n");
		printf("ע�⣺����������Ƶ������˵�����ģ�����չʾЧ����\n      �ؼ����б���û�е�������չʾ����û�˵�Ĵ��ﲻ���б��У���õ�ʲô�����\n");
		key = _getch();
		switch(key)
		{
		case '1':
			printf("1.�ƴ�Ѷ��\n");
			return "wav/�ƴ�Ѷ��.wav";
		case '2':
			printf("2.����ɽ����\n");
			return "wav/����ɽ����.wav";
		case '3':
			printf("3.��³ʯ��\n");
			return "wav/��³ʯ��.wav";
		case '4':
			printf("4.һ�����������߰˾�ʮ\n");
			return "wav/һ�����������߰˾�ʮ.wav";
		default:
			continue;
		}
	}
	exit(0);
	return NULL;
}