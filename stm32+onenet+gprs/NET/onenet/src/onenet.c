/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	onenet.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-05-08
	*
	*	�汾�� 		V1.1
	*
	*	˵���� 		��onenetƽ̨�����ݽ����ӿڲ�
	*
	*	�޸ļ�¼��	V1.0��Э���װ�������ж϶���ͬһ���ļ������Ҳ�ͬЭ��ӿڲ�ͬ��
	*				V1.1���ṩͳһ�ӿڹ�Ӧ�ò�ʹ�ã����ݲ�ͬЭ���ļ�����װЭ����ص����ݡ�
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//�����豸
#include "m6312.h"

//Э���ļ�
#include "onenet.h"
#include "mqttkit.h"

//Ӳ������
#include "usart.h"
#include "delay.h"
#include "master.h"
#include "crc16.h"
#include "bsp_adc.h"


//C��
#include <string.h>
#include <stdio.h>


#define PROID		"182370"

#define AUTH_INFO	"iSw7iiAZCXsyEWKnTpkiMhebkKs="

#define DEVID		"502129186"


extern unsigned char esp8266_buf[128];

extern __IO uint32_t ADC_ConvertedValue[NOFCHANEL];//ADC1ת���ĵ�ѹֵͨ��MDA����SRAM

float ADC_ConvertedValueLocal[NOFCHANEL*2];//�ֲ����������ڱ�������ĵ�ѹֵ


//==========================================================
//	�������ƣ�	OneNet_DevLink
//
//	�������ܣ�	��onenet��������
//
//	��ڲ�����	��
//
//	���ز�����	1-�ɹ�	0-ʧ��
//
//	˵����		��onenetƽ̨��������
//==========================================================
_Bool OneNet_DevLink(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};					//Э���

	unsigned char *dataPtr;
	
	_Bool status = 1;
	
	UsartPrintf(USART_DEBUG, "OneNet_DevLink\r\n"
							"PROID: %s,	AUIF: %s,	DEVID:%s\r\n"
                        , PROID, AUTH_INFO, DEVID);
	
	if(MQTT_PacketConnect(PROID, AUTH_INFO, DEVID, 256, 0, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
	{
		M6312_SendData(mqttPacket._data, mqttPacket._len);				//�ϴ�ƽ̨
		
		dataPtr = M6312_GetIPD(250);									//�ȴ�ƽ̨��Ӧ
		if(dataPtr != NULL)
		{
			if(MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{                   
				switch(MQTT_UnPacketConnectAck(dataPtr))
				{
					case 0:UsartPrintf(USART_DEBUG, "Tips:	���ӳɹ�\r\n");status = 0;break;
					
					case 1:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ�Э�����\r\n");break;
					case 2:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ��Ƿ���clientid\r\n");break;
					case 3:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ�������ʧ��\r\n");break;
					case 4:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ��û������������\r\n");break;
					case 5:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ��Ƿ�����(����token�Ƿ�)\r\n");break;
					default:UsartPrintf(USART_DEBUG, "ERR:	����ʧ�ܣ�δ֪����\r\n");break;
				}
			}
		}
		
		MQTT_DeleteBuffer(&mqttPacket);								//ɾ��
	}
	else
		UsartPrintf(USART_DEBUG, "WARN:	MQTT_PacketConnect Failed\r\n");
	
	return status;
	
}


//#define N 10
//  char Temps(){
//	int sum=0;
//	int count;
//	for(count=0;count<N;count++)
//	{
//		sum+=Temp.F_data;
//		DelayXms(50);
//	}
//		return (char)(sum/N);
//}
//	
// char Cods(){
//		int sum=0;
//		int count;
// for(count=0;count<N;count++)
//	{
//		sum+=Cod.F_data;
//		DelayXms(50);
//	}
//		return (char)(sum/N);
//	}
// 
//  char Tocs(){
//		int sum=0;
//		int count;
// for(count=0;count<N;count++)
//	{
//		sum+=Toc.F_data;
//		DelayXms(50);
//	}
//		return (char)(sum/N);
//	}   
//	
//  char Ntus(){
//		int sum=0;
//		int count;
// for(count=0;count<N;count++)
//	{
//		sum+=NTU.F_data;
//		DelayXms(50);
//	}
//		return (char)(sum/N);
//	}
//	
//	  char Tss(){
//		int sum=0;
//		int count;
// for(count=0;count<N;count++)
//	{
//		sum+=Turbidity.F_data;
//		DelayXms(50);
//	}
//		return (char)(sum/N);
//	}

//#define N 11
//float Temps()
//{
//	char value_buf[N];
//	char count,i,j,temp;
//	for(count=0;count<N;count++)
//	{
//		value_buf[count]=Temp.F_data;
//		DelayXms(50);
//	}
//	for(j=0;j<N-1;j++)
//	{
//		for(i=0;i<N-j;i++)
//		{
//			if(value_buf[i]>value_buf[i+1])
//			{
//				temp=value_buf[i];
//				value_buf[i]=value_buf[i+1];
//				value_buf[i+1]=temp;
//			}
//		}
//		
//	}
//	return value_buf[(N-1)/2];
//}
//	
//float Cods()
//{
//	char value_buf[N];
//	char count,i,j,cod;
//	
//	for(count=0;count<N;count++)
//	{
//		value_buf[count]=Cod.F_data;
//		DelayXms(50);
//	}
//	for(j=0;j<N-1;j++)
//	{
//		for(i=0;i<N-j;i++)
//		{
//			if(value_buf[i]>value_buf[i+1])
//			{
//				cod=value_buf[i];
//				value_buf[i]=value_buf[i+1];
//				value_buf[i+1]=cod;
//			}
//		}
//		
//	}
//	return value_buf[(N-1)/2];
//}

//float Tocs()
//{
//	char value_buf[N];
//	char count,i,j,toc;
//	for(count=0;count<N;count++)
//	{
//		value_buf[count]=Toc.F_data;
//		DelayXms(50);
//	}
//	for(j=0;j<N-1;j++)
//	{
//		for(i=0;i<N-j;i++)
//		{
//			if(value_buf[i]>value_buf[i+1])
//			{
//				toc=value_buf[i];
//				value_buf[i]=value_buf[i+1];
//				value_buf[i+1]=toc;
//			}
//		}
//		
//	}
//	return value_buf[(N-1)/2];
//}

//float Ntus()
//{
//	float value_buf[N];
//	char count,i,j,Ntu;
//	for(count=0;count<N;count++)
//	{
//		value_buf[count]=NTU.F_data;
//		DelayXms(50);
//	}
//	for(j=0;j<N-1;j++)
//	{
//		for(i=0;i<N-j;i++)
//		{
//			if(value_buf[i]>value_buf[i+1])
//			{
//				Ntu=value_buf[i];
//				value_buf[i]=value_buf[i+1];
//				value_buf[i+1]=Ntu;
//			}
//		}
//		
//	}
//	return (float)value_buf[(N-1)/2];
//}

//float Tss()
//{
//	float value_buf[N];
//	char count,i,j,Tss;
//	for(count=0;count<N;count++)
//	{
//		value_buf[count]=Turbidity.F_data;
//		DelayXms(50);
//	}
//	for(j=0;j<N-1;j++)
//	{
//		for(i=0;i<N-j;i++)
//		{
//			if(value_buf[i]>value_buf[i+1])
//			{
//				Tss=value_buf[i];
//				value_buf[i]=value_buf[i+1];
//				value_buf[i+1]=Tss;
//			}
//		}
//		
//	}
//	return (float)value_buf[(N-1)/2];
//}




unsigned char OneNet_FillBuf(char *buf)
{
	uint16_t temp0=0,temp1=0;
	
	char text[64];
	double ph;
	double tds;
//	double CODr;
//	double TOCr;
  double Bod;
	
	temp0 = (ADC_ConvertedValue[0]&0XFFFF0000) >> 16;
	temp1 = (ADC_ConvertedValue[0]&0XFFFF);
	
	ADC_ConvertedValueLocal[1] =(float) temp1/4096*3.3;
	ADC_ConvertedValueLocal[0] =(float) temp0/4096*3.3;	
	
	ph=-5.9647*ADC_ConvertedValueLocal[1]+20.255;
  tds=(153.42*ADC_ConvertedValueLocal[0]*ADC_ConvertedValueLocal[0]*ADC_ConvertedValueLocal[0]-255.86*ADC_ConvertedValueLocal[0]*ADC_ConvertedValueLocal[0]+857.39*ADC_ConvertedValueLocal[0])*1;

//	CODr=0.981*Cods()+0.013;
//	TOCr=0.40009*CODr;
  Bod=0.4*Cod.F_data;


  memset(text, 0, sizeof(text));
	strcpy(buf, ",;");
	
	memset(text, 0, sizeof(text));
	sprintf(text, "Tempreture,%.2f;",Temp.F_data);
	strcat(buf, text);
	
  memset(text, 0, sizeof(text));
	sprintf(text, "PH,%.2f;",ph);
	strcat(buf, text);
	
	memset(text, 0, sizeof(text));
	sprintf(text, "TDS,%.2f;",tds);
	strcat(buf, text);     
	
	memset(text, 0, sizeof(text));
	sprintf(text, "Cod,%.2f;",Cod.F_data);
	strcat(buf, text);
	
	memset(text, 0, sizeof(text));
	sprintf(text, "Toc,%.2f;",Toc.F_data);
	strcat(buf, text);
	
	memset(text, 0, sizeof(text));
	sprintf(text, "NTU,%.2f;",NTU.F_data);
	strcat(buf, text);
	
	memset(text, 0, sizeof(text));
	sprintf(text, "Bod,%.2f;",Bod); 
	strcat(buf, text);
	
	memset(text, 0, sizeof(text));
  sprintf(text, "Tss,%.2f;",Turbidity.F_data);
	strcat(buf, text);
	
	return strlen(buf);

}
	


//==========================================================
//	�������ƣ�	OneNet_SendData
//
//	�������ܣ�	�ϴ����ݵ�ƽ̨
//
//	��ڲ�����	type���������ݵĸ�ʽ
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNet_SendData(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};												//Э���
	
	char buf[256];
	
	short body_len = 0, i = 0;
	
	UsartPrintf(USART_DEBUG, "Tips:	OneNet_SendData-MQTT\r\n");
	
	memset(buf, 0, sizeof(buf));
	
	body_len = OneNet_FillBuf(buf);																	//��ȡ��ǰ��Ҫ���͵����������ܳ���
	
	if(body_len)
	{
		if(MQTT_PacketSaveData(DEVID, body_len, NULL, 5, &mqttPacket) == 0)							//���
		{
			for(; i < body_len; i++)
				mqttPacket._data[mqttPacket._len++] = buf[i];
			
			M6312_SendData(mqttPacket._data, mqttPacket._len);										//�ϴ����ݵ�ƽ̨
			UsartPrintf(USART_DEBUG, "Send %d Bytes\r\n", mqttPacket._len);
			
			MQTT_DeleteBuffer(&mqttPacket);															//ɾ��
		}
		else
			UsartPrintf(USART_DEBUG, "WARN:	MQTT_NewBuffer Failed\r\n");
	}
	
}

//==========================================================
//	�������ƣ�	OneNet_RevPro
//
//	�������ܣ�	ƽ̨�������ݼ��
//
//	��ڲ�����	dataPtr��ƽ̨���ص�����
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNet_RevPro(unsigned char *cmd)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};								//Э���
	
	char *req_payload = NULL;
	char *cmdid_topic = NULL;
	
	unsigned short req_len = 0;
	
	unsigned char type = 0;
	
	short result = 0;

	char *dataPtr = NULL;
	char numBuf[10];
	int num = 0;
	
	type = MQTT_UnPacketRecv(cmd);
	switch(type)
	{
		case MQTT_PKT_CMD:															//�����·�
			
			result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len);	//���topic����Ϣ��
			if(result == 0)
			{
				UsartPrintf(USART_DEBUG, "cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);
				
				if(MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqttPacket) == 0)	//����ظ����
				{
					UsartPrintf(USART_DEBUG, "Tips:	Send CmdResp\r\n");
					
					M6312_SendData(mqttPacket._data, mqttPacket._len);				//�ظ�����
					MQTT_DeleteBuffer(&mqttPacket);									//ɾ��
				}
			}
		
		break;
			
		case MQTT_PKT_PUBACK:														//����Publish��Ϣ��ƽ̨�ظ���Ack
		
			if(MQTT_UnPacketPublishAck(cmd) == 0)
				UsartPrintf(USART_DEBUG, "Tips:	MQTT Publish Send OK\r\n");
			
		break;
		
		default:
			result = -1;
		break;
	}
	
	M6312_Clear();										//��ջ���
	
	if(result == -1)
		return;
	
	dataPtr = strchr(req_payload, '}');					//����'}'

	if(dataPtr != NULL && result != -1)					//����ҵ���
	{
		dataPtr++;
		
		while(*dataPtr >= '0' && *dataPtr <= '9')		//�ж��Ƿ����·��������������
		{
			numBuf[num++] = *dataPtr++;
		}
		
		num = atoi((const char *)numBuf);				//תΪ��ֵ��ʽ
	}

	if(type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
	{
		MQTT_FreeBuffer(cmdid_topic);
		MQTT_FreeBuffer(req_payload);
	}

}
