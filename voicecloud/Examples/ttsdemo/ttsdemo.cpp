// ttsdemo.cpp : Defines the entry point for the console application.
//
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>

#include "../../include/qtts.h"


#ifdef _WIN64
#pragma comment(lib,"../../lib/msc_x64.lib")//x64
#else
#pragma comment(lib,"../../lib/msc.lib")//x86
#endif

typedef int SR_DWORD;
typedef short int SR_WORD ;

//��Ƶͷ����ʽ
struct wave_pcm_hdr
{
	char            riff[4];                        // = "RIFF"
	SR_DWORD        size_8;                         // = FileSize - 8
	char            wave[4];                        // = "WAVE"
	char            fmt[4];                         // = "fmt "
	SR_DWORD        dwFmtSize;                      // = ��һ���ṹ��Ĵ�С : 16

	SR_WORD         format_tag;              // = PCM : 1
	SR_WORD         channels;                       // = ͨ���� : 1
	SR_DWORD        samples_per_sec;        // = ������ : 8000 | 6000 | 11025 | 16000
	SR_DWORD        avg_bytes_per_sec;      // = ÿ���ֽ��� : dwSamplesPerSec * wBitsPerSample / 8
	SR_WORD         block_align;            // = ÿ�������ֽ��� : wBitsPerSample / 8
	SR_WORD         bits_per_sample;         // = ����������: 8 | 16

	char            data[4];                        // = "data";
	SR_DWORD        data_size;                // = �����ݳ��� : FileSize - 44 
} ;

//Ĭ����Ƶͷ������
struct wave_pcm_hdr default_pcmwavhdr = 
{
	{ 'R', 'I', 'F', 'F' },
	0,
	{'W', 'A', 'V', 'E'},
	{'f', 'm', 't', ' '},
	16,
	1,
	1,
	16000,
	32000,
	2,
	16,
	{'d', 'a', 't', 'a'},
	0  
};

int text_to_speech(const char* src_text ,const char* des_path ,const char* params)
{
	struct wave_pcm_hdr pcmwavhdr = default_pcmwavhdr;
	const char* sess_id = NULL;
	int ret = 0;
	unsigned int text_len = 0;
	char* audio_data;
	unsigned int audio_len = 0;
	int synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;
	FILE* fp = NULL;

	int pos = 0;			//���ڱ����һ���Ѿ��ϳɵ���λ��
	int loop_count = 0;		//���ڱ�ǣ�ȡ�˼��ν��
	int upload_flow = 0,download_flow = 0;//�ϴ���������������
	char param_value[32] = "";//����ֵ���ַ�����ʽ
	unsigned int value_len = 32;	//�ַ������Ȼ�buffer����

	printf("begin to synth...\n");
	if (NULL == src_text || NULL == des_path)
	{
		printf("params is null!\n");
		return -1;
	}
	text_len = (unsigned int)strlen(src_text);
	fp = fopen(des_path,"wb");
	if (NULL == fp)
	{
		printf("open file %s error\n",des_path);
		return -1;
	}
	sess_id = QTTSSessionBegin(params, &ret);
	if ( ret != MSP_SUCCESS )
	{
		printf("QTTSSessionBegin: qtts begin session failed Error code %d.\n",ret);
		return ret;
	}

	ret = QTTSTextPut(sess_id, src_text, text_len, NULL );
	if ( ret != MSP_SUCCESS )
	{
		printf("QTTSTextPut: qtts put text failed Error code %d.\n",ret);
		QTTSSessionEnd(sess_id, "TextPutError");
		return ret;
	}

	fwrite(&pcmwavhdr, 1, sizeof(pcmwavhdr), fp);

	while ( true )
	{
		audio_data = (char*)QTTSAudioGet( sess_id ,&audio_len , &synth_status , &ret );
		/****************��ȡ�ϳ�λ��*****************///�������Ҫ�ù��ܣ����ο���ɾ��
		const char* audio_info = QTTSAudioInfo(sess_id);	//��ȡ�Ѿ��õ�����Ƶ��λ��
		int ced = atoi(audio_info+4);						//λ����Ϣ�ĸ�ʽ��ced=xxx������ǰ�ĸ��ַ����ض�����
		char text_complete[255];							//�����ַ��������Ѿ��ϳ���ϵ��ı���
		strcpy(text_complete ,src_text + pos);					//��ԭ�ַ������Ƶ����ַ����У�������ǰ�����нضϣ���֮ǰ�ϳɵģ��Ǳ��κϳɵ��ı�����
		text_complete[ced-pos]='\0';						//�����ַ������к󲿽ضϣ��ضϵ�ced��ʾ��λ�ã���������ַ������Ǳ��λ�ȡ�����ַ�����
		printf("[%d]:get result[err==%d / status==%d]:%s\n", (loop_count), ret, synth_status,text_complete);
		loop_count++;										//ȡ�����������
		pos = ced;											//�ϳ�λ�ø���
		/**************��ȡ�ϳ�λ�ý���***************/
		if ( ret != MSP_SUCCESS )
		{
			printf("QTTSAudioGet: qtts get audio failed Error code %d.\n",ret);
			break;
		}
		/*********��ȡ��ǰ�Ự��ʱ��������Ϣ**********///�������Ҫ�ù��ܣ����ο���ɾ��
		value_len = 32;//value_len���Ǵ�����������Ǵ���������ÿ�ε���QTTSGetParamʱҪ����Ϊbuffer����
		ret = QTTSGetParam(sess_id,"upflow",param_value,&value_len);//��ȡ����������Ϣ
		if ( ret != MSP_SUCCESS )
		{
			printf("QTTSGetParam: qtts get param failed Error code %d.\n",ret);
			break;
		}
		upload_flow = atoi(param_value);
		printf("upflow== %d Byte\n",upload_flow);
		value_len = 32;//value_len���Ǵ�����������Ǵ���������ÿ�ε���QTTSGetParamʱҪ����Ϊbuffer����
		ret = QTTSGetParam(sess_id,"downflow",param_value,&value_len);//��ȡ����������Ϣ
		if ( ret != MSP_SUCCESS )
		{
			printf("QTTSGetParam: qtts get param failed Error code %d.\n",ret);
			break;
		}
		download_flow = atoi(param_value);
		printf("downflow== %d Byte\n",download_flow);
		/**************��ȡ������Ϣ����***************/
		fwrite(audio_data, 1, audio_len, fp);
		pcmwavhdr.data_size += audio_len;//����pcm���ݵĴ�С
		if ( MSP_TTS_FLAG_DATA_END == synth_status )
		{
			printf("QTTSAudioGet: get end of data.\n");
			break;
		}
	}
	//����pcm�ļ�ͷ���ݵĴ�С
	pcmwavhdr.size_8 += pcmwavhdr.data_size + 36;

	//��������������д���ļ�ͷ��
	fseek(fp, 4, 0);
	fwrite(&pcmwavhdr.size_8,sizeof(pcmwavhdr.size_8), 1, fp);
	fseek(fp, 40, 0);
	fwrite(&pcmwavhdr.data_size,sizeof(pcmwavhdr.data_size), 1, fp);

	fclose(fp);

	ret = QTTSSessionEnd(sess_id, "Normal");
	if ( ret != MSP_SUCCESS )
	{
		printf("QTTSSessionEnd: qtts end failed Error code %d.\n",ret);
	}
	return ret;
}

int _tmain(int argc, _TCHAR* argv[])
{
	///APPID��������Ķ�
	const char* m_configs = "appid=52d8f781";
	const char* text1  = "Ѷ����������ͨ���ޡ�";
	const char*  filename1 = "text_to_speech_test_1.wav";
	const char* param1 = "ssm=1,auf=audio/L16;rate=16000,vcn=xiaoyan";
	const char* text2  = "��ӭʹ�ã����տƴ�Ѷ����Ϣ�Ƽ��ɷ����޹�˾���������ϳɡ�";
	const char*  filename2 = "text_to_speech_test_2.wav";
	const char* param2 = "ssm=1,auf=audio/L16;rate=16000,vcn=xiaoyu";
	int ret = 0;
	char key = 0;

	//�����ʼ��
	ret = QTTSInit( m_configs);
	if ( ret != MSP_SUCCESS )
	{
		printf("QTTSInit: failed, Error code %d.\n", ret);
		key = _getch();
		return ret;
	}
	//�ϳ��ı�
	ret = text_to_speech(text1,filename1,param1);
	if ( ret != MSP_SUCCESS )
	{
		printf("text_to_speech: failed , Error code %d.\n",ret);
	}
	ret = text_to_speech(text2,filename2,param2);
	if ( ret != MSP_SUCCESS )
	{
		printf("text_to_speech: failed , Error code %d.\n",ret);
	}
	//����ر�
	ret = QTTSFini();
	if ( ret != MSP_SUCCESS )
	{
		printf("QTTSFini: failed , Error code %d.\n",ret);
	}
	else
	{
		printf("Complete!\nPress any key to exit.\n");
	}
	key = _getch();
	return 0;
}

