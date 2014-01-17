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



const char* get_grammar( const char* filename );//获取语法
void release_grammar(const char** grammar);//释放语法占用的空间
int run_asr(const char* grammar , const char* asrfile);//测试识别效果
const char*  get_audio_file(void);//选择音频文件

const int BUFFER_NUM = 4096;
const int MAX_KEYWORD_LEN = 4096;

int _tmain(int argc, _TCHAR* argv[])
{
	const char* grammar = NULL;
	const char* asrfile = get_audio_file();
	int ret = MSP_SUCCESS;
	//appid 请勿随意改动
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

	grammar = (char*)malloc(file_len+1); //从文件中读取语法，注意文本编码为GB2312
	fread((void*)grammar,1,file_len,fp);
	grammar[file_len]='\0'; //字符串收尾
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

	const char* param = "rst=plain,sub=asr,ssm=1,aue=speex,auf=audio/L16;rate=16000,grt=abnf";//注意sub=asr,grt=abnf
	const char* sess_id = QISRSessionBegin(grammar, param, &ret);//将语法传入QISRSessionBegin
	if ( MSP_SUCCESS != ret )
	{
		printf("QISRSessionBegin err %d\n", ret);	
		return ret;
	}

	fp = fopen( asrfile , "rb");//我们提供了几个音频文件，测试时根据需要在这里更换
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
		feof(fp) ? status = MSP_AUDIO_SAMPLE_LAST : status = MSP_AUDIO_SAMPLE_CONTINUE;//最后一块音频要使用last
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
		Sleep(200);//因为是模拟录音，为了避免数据发送太快造成缓冲区溢出，所以这里暂停200ms，如果是实时录音，不必暂停
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
	while(key != 27)//按Esc则退出
	{
		system("cls");//清屏
		printf("请选择音频文件：\n");
		printf("1.055165309093\n");
		printf("2.18012345678\n");
		printf("3.邮编230088\n");
		printf("4.安徽科大讯飞信息科技股份有限公司\n");
		printf("注意：第三条的音频是文字和数字的混合，文字部分也在尽可能地向数字靠，置信度低。\n      第四条没有数字，被认为是噪音。\n");
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
			printf("3.邮编230088\n");
			return "wav/邮编230088.wav";
		case '4':
			printf("4.安徽科大讯飞信息科技股份有限公司\n");
			return "wav/安徽科大讯飞信息科技股份有限公司.wav";
		default:
			continue;
		}
	}
	exit(0);
	return NULL;
}