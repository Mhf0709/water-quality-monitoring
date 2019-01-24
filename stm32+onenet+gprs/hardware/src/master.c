#include "master.h"
#include "delay.h"
#include "crc16.h"

#define modbus_time 40

u8 RS485_RX_BUFF[100];												//���ջ�����100�ֽ�
u16 RS485_RX_CNT=0;														//���ռ�����
u8 RS485_RxFlag=0;														//����һ֡�������

u8 RS485_TX_BUFF[8];													//���ͻ�����
u16 RS485_TX_CNT=0;														//���ͼ�����
u8 RS485_TxFlag=0;														//����һ֡�������


u8 TX_RX_SET=0; //���ͣ����������л��� 0 ����ģʽ 1����ģʽ
u8 ComErr=8; //0����ͨѶ����
             //1����CRC����
						// 2�����������

RS_485_DATA Temp,Cod,Toc,NTU;		//�¶�    COD  TOC  �Ƕ�

RS_485_DATA Turbidity;	//���Ƕ�


//��ʼ��USART3
void RS485_Init(u32 bound)
{
        GPIO_InitTypeDef GPIO_InitStructure;
        USART_InitTypeDef USART_InitStructure;
        NVIC_InitTypeDef NVIC_InitStructure;
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO,ENABLE);					//ʹ��GPIOBʱ��
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);					//ʹ�ܴ���3ʱ��
        
        GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;											//PB10��TX�������������
        GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
        GPIO_Init(GPIOB,&GPIO_InitStructure);
        GPIO_SetBits(GPIOB,GPIO_Pin_10);															//Ĭ�ϸߵ�ƽ
        
        GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11;												//PB11��RX����������
        GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;   		//GPIO_Mode_IN_FLOATING(��������)/////////////////////////////////////////////
        GPIO_Init(GPIOB,&GPIO_InitStructure);
        
//        GPIO_InitStructure.GPIO_Pin=GPIO_Pin_12;									//ͨ���������->PB12��RE/DE��ͨ���������//////////////////////////////////////////////////////////////////////
//        GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
//        GPIO_Init(GPIOB,&GPIO_InitStructure);
//        GPIO_ResetBits(GPIOB,GPIO_Pin_12);//Ĭ�Ͻ���״̬
        
        USART_DeInit(USART3);																				//��λ����3
        USART_InitStructure.USART_BaudRate=bound;
        USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
        USART_InitStructure.USART_WordLength=USART_WordLength_8b;
        USART_InitStructure.USART_StopBits=USART_StopBits_1;
        USART_InitStructure.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;								//�շ�ģʽ

        USART_InitStructure.USART_Parity=USART_Parity_No;//��У��
        USART_Init(USART3,&USART_InitStructure);													//��ʼ������3
        
        USART_ClearITPendingBit(USART3,USART_IT_RXNE);									//�������3�����жϱ�־
        USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);										//ʹ�ܴ���3�����ж�
        
        NVIC_InitStructure.NVIC_IRQChannel=USART3_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority=2;
        NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        
        USART_Cmd(USART3,ENABLE);																					//ʹ�ܴ���3
//        RS485_TX_EN=1;//Ĭ��Ϊ����ģʽ
        
        Timer7_Init();//��ʱ��7��ʼ�������ڼ��ӿ���ʱ��
}

///////////////////////////////////////////////////////////////////////////////////////////////
////��ʱ��7��ʼ��---���ܣ��жϴӻ����ص������Ƿ�������

void Timer7_Init(void)
{
        TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
        NVIC_InitTypeDef NVIC_InitStructure;

        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE); //TIM7ʱ��ʹ�� 

        //TIM7��ʼ������
				TIM_TimeBaseStructure.TIM_Period = modbus_time; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ��4ms��Tou=((arr+1)*(psc+1))/Tclk;Tclk:����72000000
        TIM_TimeBaseStructure.TIM_Prescaler =7199; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ ���ü���Ƶ��Ϊ10kHz
        TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
        TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

        TIM_ITConfig( TIM7, TIM_IT_Update, ENABLE );//TIM7 ��������ж�

        //TIM7�жϷ�������
        NVIC_InitStructure.NVIC_IRQChannel =TIM7_IRQn;  //TIM7�ж�
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //��ռ���ȼ�2��
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
        NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���                                                                  
}






///////////////////////////////////////////////////////////////////////////////////
void USART3_IRQHandler(void)//����2�жϷ������
{
	   
        u8 res;
        u8 err;
	 
        if(USART_GetITStatus(USART3,USART_IT_RXNE)!=RESET)
        {
                if(USART_GetFlagStatus(USART3,USART_FLAG_NE|USART_FLAG_FE|USART_FLAG_PE)) err=1;//��⵽������֡�����У�����
                else err=0;
                res=USART_ReceiveData(USART3); 																	//�����յ����ֽڣ�ͬʱ��ر�־�Զ����
                
                if((RS485_RX_CNT<100)&&(err==0))
                {
                        RS485_RX_BUFF[RS485_RX_CNT]=res;
                        RS485_RX_CNT++;
                        
                        TIM_ClearITPendingBit(TIM7,TIM_IT_Update);								//�����ʱ������ж�
                        TIM_SetCounter(TIM7,0);																		//�����յ�һ���µ��ֽڣ�����ʱ��7��λΪ0�����¼�ʱ���൱��ι����
                        TIM_Cmd(TIM7,ENABLE);																			//��ʼ��ʱ
                }
        }
}

/////////////////////////////////////////////////////////////////////////////////////

//�ö�ʱ��7�жϽ��տ���ʱ�䣬������ʱ�����ָ��ʱ�䣬��Ϊһ֡����
//��ʱ��7�жϷ������         
void TIM7_IRQHandler(void)
{                                                                   
        if(TIM_GetITStatus(TIM7,TIM_IT_Update)!=RESET)
        {
                TIM_ClearITPendingBit(TIM7,TIM_IT_Update);//����жϱ�־
                TIM_Cmd(TIM7,DISABLE);//ֹͣ��ʱ��
 //               RS485_TX_EN=1;//ֹͣ���գ��л�Ϊ����״̬
                RS485_RxFlag=1;//��λ֡�������
        }
}

////////////////////////////////////////////////////////////////////////////
//����n���ֽ�����
//buff:�������׵�ַ
//len�����͵��ֽ���
void RS485_SendData(u8 *buff,u8 len)
{ 
 //       RS485_TX_EN=1;//�л�Ϊ����ģʽ
        while(len--)
        {
                while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)==RESET);					//�ȴ�������Ϊ��
                USART_SendData(USART3,*(buff++));
        }
        while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET);								//�ȴ��������
				TX_RX_SET=1;																													 //����������ɣ���ʱ��T4������յ�������
	//			RS485_TX_EN=0;
}



//Modbus������03�������///////////////////////////////////////////////////////////////////////////////////////
//�����ּĴ���
void Master_03_Slove(u8 board_adr,u8 function_03,u16 start_adress,u16 num)//��������ַ  ������ �Ĵ�����ʼ��ַ ��ȡ����
{ 	
			u16 calCRC;
	
		RS485_TX_BUFF[0] = board_adr;														//��������ַ  
		RS485_TX_BUFF[1 ]= function_03;													//������ 
    RS485_TX_BUFF[2] = HI(start_adress);  									//�Ĵ�����ʼ��ַ  ��8λ
    RS485_TX_BUFF[3] = LOW(start_adress); 									//�Ĵ�����ʼ��ַ  ��8λ
    RS485_TX_BUFF[4] = HI(num);															//��ȡ���� ��8λ
    RS485_TX_BUFF[5] = LOW(num);														//��ȡ���� ��8λ
    calCRC=CRC_Compute(RS485_TX_BUFF,6);										//����CRC16У����
    RS485_TX_BUFF[6]=(calCRC>>8)&0xFF;										//CRC��8λ		��ǰ
    RS485_TX_BUFF[7]=(calCRC)&0xFF;												//CRC��8λ		�ں�
//	  RS485_TX_BUFF[7]=(calCRC>>8)&0xFF;											//CRC��8λ	�ں�
//    RS485_TX_BUFF[6]=(calCRC)&0xFF;													//CRC��8λ    ��ǰ
	
	RS485_SendData(RS485_TX_BUFF,8);	
}


void Master_Service(u8 board_adr,u8 function,u16 start_adress,u16 num)//��������ַ  ������ �Ĵ�����ʼ��ַ ��ȡ����
{

	switch(function)
	{
		case 03:
				Master_03_Slove(board_adr,function,start_adress,num);
				break;
	}
}
#include "usart.h"
/////////////////////////////////////////////////////////////////////////////////////
//RS485����������ڴ�����յ�������(������������ѭ������)

void RS485_RX_Service(void)
{
		u16 calCRC;
        u16 recCRC;
        if(RS485_RxFlag==1)
        {
//#ifdef Debug
//	 printf("\r\n�ҽ��յ���һ֡����\r\n");
//#endif	
//#ifdef Debug
//	 printf("\r\n����Ϊ\r\n");
//	usart1_SendData(RS485_RX_BUFF,19);
//#endif	
                if(RS485_RX_BUFF[0]==0x01||RS485_RX_BUFF[0]==0x02||RS485_RX_BUFF[0]==0x03||RS485_RX_BUFF[0]==0x04||RS485_RX_BUFF[0]==0x05||RS485_RX_BUFF[0]==0x06||RS485_RX_BUFF[0]==0x07|RS485_RX_BUFF[0]==0x08)//��ַ��ȷ
                {
                        if((RS485_RX_BUFF[1]==01)||(RS485_RX_BUFF[1]==02)||(RS485_RX_BUFF[1]==03)||(RS485_RX_BUFF[1]==05)||(RS485_RX_BUFF[1]==06)||(RS485_RX_BUFF[1]==15)||(RS485_RX_BUFF[1]==16))//��������ȷ
												{
                                        calCRC=CRC_Compute(RS485_RX_BUFF,RS485_RX_CNT-2);//�������������ݵ�CRC
                                        recCRC=RS485_RX_BUFF[RS485_RX_CNT-1]|(((u16)RS485_RX_BUFF[RS485_RX_CNT-2])<<8);//���յ���CRC(���ֽ���ǰ�����ֽ��ں�)
                                        if(calCRC==recCRC)//CRCУ����ȷ
                                        {
                                                /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                switch(RS485_RX_BUFF[1])//���ݲ�ͬ�Ĺ�������д���
                                                {                                                                
																										case 03: //������Ĵ���															
																														Modbus_03_Solve();
																														break;                                                                                       
                                                }
																				}
                                        else//CRCУ�����
                                        {
												  ComErr=14;

                                        }                                                                                       
												}
                        else//���������
                        {
//#ifdef Debug
//	 printf("\r\n��û�нӵ�����\r\n");
//	usart1_SendData(RS485_RX_BUFF,19);
//#endif
														if((RS485_RX_BUFF[1]==0x81)||(RS485_RX_BUFF[1]==0x82)||(RS485_RX_BUFF[1]==0x83)||(RS485_RX_BUFF[1]==0x85)||(RS485_RX_BUFF[1]==0x86)||(RS485_RX_BUFF[1]==0x8F)||(RS485_RX_BUFF[1]==0x90))
														{
															switch(RS485_RX_BUFF[2])
															{
																case 0x01:
																			ComErr=11;
																			break;
																case 0x02:
																			ComErr=12;
																			break;
																case 0x03:
																			ComErr=13;
																			break;
																case 0x04:
																			ComErr=14;
																			break;
																
															}
															TX_RX_SET=0; //�������	
														}
                        }
          }
                                
				RS485_RxFlag=0;//��λ֡������־
				RS485_RX_CNT=0;//���ռ��������� 
				TX_RX_SET=0; //�������
        }                
}


//Modbus������03�������///////////////////////////////////////////////////////////////////////////////////////����֤����OK
//�����ּĴ���

void Modbus_03_Solve(void)			//���մ���
{
	switch(RS485_RX_BUFF[0])
	{
		case 1:
					if(RS485_RX_BUFF[2]==0x0E)			//�¶�  COD  TOC
					{
							Temp.dat[0]=RS485_RX_BUFF[3];			//�¶�
							Temp.dat[1]=RS485_RX_BUFF[4];
							Temp.dat[2]=RS485_RX_BUFF[5];
							Temp.dat[3]=RS485_RX_BUFF[6];
						
							Cod.dat[0]=RS485_RX_BUFF[7];			//COD
							Cod.dat[1]=RS485_RX_BUFF[8];
							Cod.dat[2]=RS485_RX_BUFF[9];
							Cod.dat[3]=RS485_RX_BUFF[10];
						
							Toc.dat[0]=RS485_RX_BUFF[11];			//TOC	
							Toc.dat[1]=RS485_RX_BUFF[12];
							Toc.dat[2]=RS485_RX_BUFF[13];
							Toc.dat[3]=RS485_RX_BUFF[14];
					}
					else if(RS485_RX_BUFF[2]==0x04)		    //�Ƕ�
					{
							NTU.dat[0]=RS485_RX_BUFF[3];			//�¶�
							NTU.dat[1]=RS485_RX_BUFF[4];
							NTU.dat[2]=RS485_RX_BUFF[5];
							NTU.dat[3]=RS485_RX_BUFF[6];
					}
						break;
		case 2:
					if(RS485_RX_BUFF[2]==0x08)
					{
						Turbidity.dat[0]=RS485_RX_BUFF[7];			//���Ƕ���һ��������
						Turbidity.dat[1]=RS485_RX_BUFF[8];
						Turbidity.dat[2]=RS485_RX_BUFF[9];
						Turbidity.dat[3]=RS485_RX_BUFF[10];
					}
					break;
	}
		TX_RX_SET=0; //�������
}




