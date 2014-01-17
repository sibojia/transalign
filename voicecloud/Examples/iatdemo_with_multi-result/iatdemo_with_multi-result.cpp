// iatdemo_with_multi-result.cpp : Defines the entry point for the console application.
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

const int BUFFER_NUM = 1024 * 4;
const int AMR_HEAD_SIZE = 6;

void run_iat(const char* src_wav_filename , const char* des_text_filename , const char* param)
{
	bool error = false;
	int ret = MSP_SUCCESS;
	int i = 0;
	FILE* fp = NULL;
	FILE* fout = NULL;
	char buff[BUFFER_NUM];
	unsigned int len;
	int status = MSP_AUDIO_SAMPLE_CONTINUE, ep_status = -1, rec_status = -1, rslt_status = -1;

	///�ڶ�������Ϊ���ݵĲ�����ʹ�ûỰģʽ��ʹ��speex����룬ʹ��16k16bit����Ƶ����
	///����������Ϊ������
	const char* sess_id = QISRSessionBegin(NULL, param, &ret);
	if ( MSP_SUCCESS != ret )
	{
		printf("QISRSessionBegin err %d\n", ret);	
		error = true;
	}

	///ģ��¼����������Ƶ
	if (error == false)
	{
		fp = fopen( src_wav_filename , "rb");
		if ( NULL == fp )
		{
			printf("failed to open file,please check the file.\n");
			error = true;
		}
	}

	///���������ļ�
	if (error == false)
	{
		fout = fopen( des_text_filename , "ab");
		if( NULL == fout )
		{
			printf("failed to open file,please check the file.\n");
			error = true;
		}
	}
	if (error == false)
	{
		printf("writing audio...\n");

		char param_value[32] = "";//����ֵ���ַ�����ʽ
		unsigned int value_len = 32;	//�ַ������Ȼ�buffer����
		int volume = 0;//������ֵ

		while ( !feof(fp) )
		{
			len = (unsigned int)fread(buff, 1, BUFFER_NUM, fp);
			printf(".");
			feof(fp) ? status = MSP_AUDIO_SAMPLE_LAST : status = MSP_AUDIO_SAMPLE_CONTINUE;//���һ����ƵҪʹ��last
			///��ʼ�������������Ƶ����
			ret = QISRAudioWrite(sess_id, buff, len, status, &ep_status, &rec_status);
			if ( ret != MSP_SUCCESS )
			{
				printf("\nQISRAudioWrite err %d\n", ret);
				error = true;
				break;
			}
			/*********��ȡ��ǰ������Ƶ��������Ϣ**********/
			value_len = 32;//value_len���Ǵ�����������Ǵ���������ÿ�ε���QTTSGetParamʱҪ����Ϊbuffer����
			ret = QISRGetParam(sess_id,"volume",param_value,&value_len);//��ȡ������Ϣ����ȡ�����������������������Ӽ�ttsdemo
			if ( ret != MSP_SUCCESS )
			{
				printf("QISRGetParam: qisr get param failed Error code %d.\n",ret);
				char key = _getch();
				break;
			}
			volume = atoi(param_value);//��ȡ����������Ϣ���������ڽ������ò�ͬ��ͼƬչʾ��̬Ч��
			//printf("volume== %d \n",volume);
			for (int i=0;i<volume;i++)
			{
				printf(".");
			}
			printf("\n");
			/*******��ȡ��ǰ������Ƶ��������Ϣ����**********/
			if (ep_status == MSP_EP_AFTER_SPEECH)//��⵽��Ƶ��˵�
			{
				printf("QISRAudioWrite: ep_status == MSP_EP_AFTER_SPEECH.\n");
				break;
			}

			///���������ز��ֽ��
			if ( rec_status == MSP_REC_STATUS_SUCCESS )
			{
				const char* result = QISRGetResult(sess_id, &rslt_status, 0, &ret);
				if( rslt_status == MSP_REC_STATUS_NO_MATCH )
					printf("get result nomatch\n");
				else
				{
					if ( result != NULL )
						fwrite(result, 1, strlen(result), fout);
					printf("get result[%d/%d]: %s\n", ret, rslt_status, result);
				}
			}
			Sleep(200);//��Ϊ��ģ��¼����Ϊ�˱������ݷ���̫����ɻ��������������������ͣ200ms�������ʵʱ¼����������ͣ
		}
		printf("\n");
		fclose(fp); 
	}

	///���һ�����ݷ���֮��ѭ���ӷ������˻�ȡ���
	///���ǵ����绷�����õ�����£����Զ�ѭ���������޶�
	if (error == false)
	{	
		printf("get reuslt\n");
		int loop_count = 0;
		do 
		{
			const char* result = QISRGetResult(sess_id, &rslt_status, 0, &ret);
			if ( ret != MSP_SUCCESS )
			{
				printf("QISRGetResult err %d\n", ret);
				error = true;
				break;
			}

			if( rslt_status == MSP_REC_STATUS_NO_MATCH )
				printf("get result nomatch\n");
			else
			{
				if ( result != NULL )
					fwrite(result, 1, strlen(result), fout);
				printf("[%d]:get result[%d/%d]: %s\n", (loop_count), ret, rslt_status, result);
			}
			Sleep(500);
		} while (rslt_status != MSP_REC_STATUS_COMPLETE && loop_count++ < 30);
	}

	if( NULL != fout )
	{
		const char* result = "\r\n";
		fwrite(result, 1, strlen(result), fout);
		fclose(fout);
	}

	ret = QISRSessionEnd(sess_id, NULL);
	if ( ret != MSP_SUCCESS )
	{
		printf("QISRSessionEnd err %d\n", ret);
		return;
	}
	printf("QISRSessionEnd.\n");
	return;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int ret = MSP_SUCCESS;
	const char* param1 = "sub=iat,ssm=1,auf=audio/L16;rate=16000,aue=speex,ent=sms16k,nbest=5";//���5����ѡ���ʽֻ��Ϊjson������ֻ��Ϊutf8
	const char* param2 = "sub=iat,ssm=1,auf=audio/L16;rate=16000,aue=speex,ent=sms16k,nbest=2";//���2����ѡ���ʽֻ��Ϊjson������ֻ��Ϊutf8,nbest��ȡֵ��ΧΪ1~5
	const char* output_file = "iat_result.txt";
	///�����ʼ����ֻ���ʼ��һ��
	///APPID��������Ķ�
	ret = QISRInit("appid=52d8f781");

	//��ʼһ·תд�Ự
	run_iat("wav/һ�����������߰˾�ʮ.wav" , output_file , param1);
	run_iat("wav/һ�����������߰˾�ʮ.wav" , output_file , param2);

	//���ʼ�����ڳ������ǰ����һ�μ���
	ret = QISRFini();
	printf("Press any key to exit.");
	char key = _getch();
	return 0;
}

