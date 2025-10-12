#include "sys.h"
#include "usart.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "DMA.h"
#include "oled1.h"
char rxdatabufer;
u16 point1 = 0;
_SaveData Save_Data;
_SaveData1 Save_Data1;  //gps��

char Receive_Buff[256];
uint8_t countA;
FlagStatus DMASign; //WIFI��
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif
 

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 
int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}


char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)              //ע�⣡��������������ĳ���uart3��printf
{      
	while((USART3->SR&0X40)==0);//ѭ������,ֱ���������   
    USART3->DR = (u8) ch;      
	return ch;
}
#endif 

/*ʹ��microLib�ķ���*/
 /* 
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}
int GetKey (void)  { 

    while (!(USART1->SR & USART_FLAG_RXNE));

    return ((int)(USART1->DR & 0x1FF));
}
*/
 
#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN]={0};     //���ջ���,���USART_REC_LEN���ֽ�.
char USART_RX_BUF3[USART_REC_LEN]={0};
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	  
  
void uart_init(u32 bound){
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
  //USART1_RX	  GPIOA.10��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

  //Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 

}

void uart2_init(u32 My_BaudRate)
{
	GPIO_InitTypeDef GPIO_InitStrue;
	USART_InitTypeDef USART_InitStrue;
	
	// ����ʹ��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
//	USART_DeInit(USART2);  //��λ����2 -> ����û��
	
	// ��ʼ�� ���ڶ�ӦIO��  TX-PA2  RX-PA3
	GPIO_InitStrue.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStrue.GPIO_Pin=GPIO_Pin_2;
	GPIO_InitStrue.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStrue);
	
	// ��ʼ�� ����ģʽ״̬
	USART_InitStrue.USART_BaudRate=My_BaudRate; // ������
	USART_InitStrue.USART_HardwareFlowControl=USART_HardwareFlowControl_None; // Ӳ��������
	USART_InitStrue.USART_Mode=USART_Mode_Tx; // ���� ģʽ
	USART_InitStrue.USART_Parity=USART_Parity_No; // û����żУ��
	USART_InitStrue.USART_StopBits=USART_StopBits_1; // һλֹͣλ
	USART_InitStrue.USART_WordLength=USART_WordLength_8b; // ÿ�η������ݿ��Ϊ8λ
	USART_Init(USART2,&USART_InitStrue);
	
	USART_Cmd(USART2,ENABLE);//ʹ�ܴ���
}

void uart3_init(u32 bound)
{  
 
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);	// GPIOBʱ��      ������ʱ�����PC10,11���ţ��ض��幦�ܣ���������������
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE); //����3ʱ��ʹ��
 
   
// 	USART_DeInit(USART3);  //��λ����3
		 //USART3_TX   PB10
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB10
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOC, &GPIO_InitStructure); //��ʼ��PB10
   
    //USART3_RX	  PB11
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOC, &GPIO_InitStructure);  //��ʼ��PB11
    
	 GPIO_PinRemapConfig(GPIO_PartialRemap_USART3,ENABLE);   //������ӳ�䣡����������������������������
     
	USART_InitStructure.USART_BaudRate = bound;//������һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
  
	USART_Init(USART3, &USART_InitStructure); //��ʼ������	3
  
 
	USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ��� 
	
	//ʹ�ܽ����ж�
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�����ж�   
	
	//�����ж����ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
}

//void USART1_IRQHandler(void)                	//����1�жϷ������
//	{
//	u8 Res;
//#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
//	OSIntEnter();    
//#endif
//	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
//		{
//		Res =USART_ReceiveData(USART1);	//��ȡ���յ�������
//		
//		if((USART_RX_STA&0x8000)==0)//����δ���
//			{
//			if(USART_RX_STA&0x4000)//���յ���0x0d
//				{
//				if(Res!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
//				else USART_RX_STA|=0x8000;	//��������� 
//				}
//			else //��û�յ�0X0D
//				{	
//				if(Res==0x0d)USART_RX_STA|=0x4000;
//				else
//					{
//					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
//					USART_RX_STA++;
//					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
//					}		 
//				}
//			}   		 
//     } 
//#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
//	OSIntExit();  											 
//#endif
//} 
    
void UART_init()
{
    uart_init(115200);
    uart2_init(9600);
    uart3_init(9600);
}    
void UART1_SendByte(uint8_t Byte)
{
	USART_SendData(USART1, Byte);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
}
void UART2_SendByte(uint8_t Byte)
{
	USART_SendData(USART2, Byte);
	while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) == RESET);
}

void USART1_IRQHandler(void)					//DMA+���ڿ����ж�
{
	if(USART_GetITStatus(USART1, USART_IT_IDLE) == SET)     //��������һ��������Ϣ
	{
		countA = USART_ReceiveData(USART1);                 //����������������������������
		USART_ClearITPendingBit(USART1,USART_IT_IDLE);      //������ڿ����жϱ�־λ
		if(Receive_Buff[0]=='+')DMASign = SET;              //Receive_Buff[0]����������ж��Ƿ���յ�������Ϣ������ǣ�DMASign��1
		DMA_Cmd(DMA1_Channel5, DISABLE);					//����һ��DMA����֤ÿ�ζ���BUFF�ĵ�һλ��ʼ
        
		DMA_ReStart((uint32_t)Receive_Buff);                //һֱ��DR�Ĵ�����ȡ256����ͬ���ֽڵ�Receive_Buff���飿������
        
	}else if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)	//������WiFi��������1���ٰ�����װ��Receive_Buff����
	{
		Receive_Buff[countA] = USART_ReceiveData(USART1);
		countA++;
	}
}
  
void USART3_IRQHandler(void)                	//����3�жϷ������
{
	u8 Res;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) 
	{
		Res =USART_ReceiveData(USART3);//(USART1->DR);	//��ȡ���յ�������
	
	if(Res == '$')
	{
		point1 = 0;	
	}
		

	  USART_RX_BUF3[point1++] = Res;

	if(USART_RX_BUF3[0] == '$' && USART_RX_BUF3[4] == 'M' && USART_RX_BUF3[5] == 'C')			//ȷ���Ƿ��յ�"GPRMC/GNRMC"��һ֡����
	{
		if(Res == '\n')									   
		{
			memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);      //���
			memcpy(Save_Data.GPS_Buffer, USART_RX_BUF3, point1); 	//��������
			Save_Data.isGetData = true;
			point1 = 0;
			memset(USART_RX_BUF3, 0, USART_REC_LEN);      //���				
		}	
				
	}
	
    if(USART_RX_BUF3[0] == '$' && USART_RX_BUF3[1] == 'G' && USART_RX_BUF3[2] == 'P' && USART_RX_BUF3[3] == 'G' && USART_RX_BUF3[9] == '1')			//ע��USART_RX_BUF[9] == '1'��ΪʲôҪ��ôд����Ϊ���ֻ�����ȡ��γ�ȷ�ʽ����д��main�����������ݸ����ٶȱȲ��ϴ����жϷ���������ȡ����Ϣ��������GPGSV�ĵڶ���������������ǵ�һ��
	{
		if(Res == '\n')									   
		{
			memset(Save_Data1.GPS_Buffer1, 0, GPS_Buffer_Length);      //���
			memcpy(Save_Data1.GPS_Buffer1, USART_RX_BUF3, point1); 	//��������
			Save_Data1.isGetData1 = true;
			point1 = 0;
			memset(USART_RX_BUF3, 0, USART_REC_LEN);      //���				
		}	
				
	}
    
	if(point1 >= USART_REC_LEN)
	{
		point1 = USART_REC_LEN;
	}	
		
   } 
}
void Serial_SendString(char Strings[],FlagStatus LineBreak) //LineBreak���Ƿ���ӻ���
{
		char *point = Strings;
		if(LineBreak == SET)
		{
			while(*point != '\0')
			{
				USART_SendData(USART1, *point);
				while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
				point++;
			}
			USART_SendData(USART1, '\r');while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
			USART_SendData(USART1, '\n');while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
		}else
		{
			while(*point != '\0')
			{
				USART_SendData(USART1, *point);
				while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
				point++;
			}
		}
}
void Serial_SendMessage(char *fmt,...)
{
  char buff[256] = {0};
  char *point = buff;
  va_list ap; 
  char *s;
  char flag;
  va_start(ap,fmt);
  while (*fmt)
  {
		flag=*fmt++;
		if(flag!='%')
		{
			*point++ = flag;
			continue;
		}
		flag=*fmt++;
    switch (flag) 
		{ 
			case 's':
			s=va_arg(ap,char*);
			while(*s != '\0')
			{
				*point++ = *s++;
			}
			break;
			case 'd':
			*point++ = va_arg(ap,int)+'0';
		}   
  }
  Serial_SendString(buff,SET);
  va_end(ap);
}
/*ע������������������Ϊ�˴���3*/
u8 Hand(char *a)                   // ��������ʶ����
{ 
    if(strstr(USART_RX_BUF3,a)!=NULL)
	    return 1;
	else
		return 0;
}

void CLR_Buf(void)                           // ���ڻ�������
{
	memset(USART_RX_BUF3, 0, USART_REC_LEN);      //���
    point1 = 0;                    
}

void clrStruct()
{
	Save_Data.isGetData = false;
	Save_Data.isParseData = false;
	Save_Data.isUsefull = false;
	memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);      //���
	memset(Save_Data.UTCTime, 0, UTCTime_Length);
	memset(Save_Data.latitude, 0, latitude_Length);
	memset(Save_Data.N_S, 0, N_S_Length);
	memset(Save_Data.longitude, 0, longitude_Length);
	memset(Save_Data.E_W, 0, E_W_Length);
	
}
#endif	

