#ifndef __MASTER_H
#define __MASTER_H
#include "sys.h"

#define READ_COIL     01
#define READ_DI       02
#define READ_HLD_REG  03
#define READ_AI       04
#define SET_COIL      05
#define SET_HLD_REG   06
#define WRITE_COIL    15
#define WRITE_HLD_REG 16


#define HI(n) ((n)>>8)
#define LOW(n) ((n)&0xff)



typedef union
{
		u8 dat[4];
		float F_data;
}RS_485_DATA;
//#define RS485_TX_EN PBout(12)		//0������ ��1������


extern RS_485_DATA Temp,Cod,Toc,NTU;//�¶�  COD  TOC
extern RS_485_DATA Turbidity;//���Ƕ�//���Ƕ���һ��������
extern u8 TX_RX_SET; //���ͣ����������л��� 0 ����ģʽ 1����ģʽ



void RS485_Init(u32 bound);
void Timer4_enable(u16 arr);


void Modbus_03_Solve(void);

void Timer7_Init(void);
void Master_Service(u8 board_adr,u8 function,u16 start_adress,u16 num);	//��������ַ  ������ �Ĵ�����ʼ��ַ ��ȡ����
void RS485_RX_Service(void);
#endif









