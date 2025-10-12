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
_SaveData1 Save_Data1;  //gps用

char Receive_Buff[256];
uint8_t countA;
FlagStatus DMASign; //WIFI用
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif
 

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
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
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)              //注意！！！！！！这里改成了uart3的printf
{      
	while((USART3->SR&0X40)==0);//循环发送,直到发送完毕   
    USART3->DR = (u8) ch;      
	return ch;
}
#endif 

/*使用microLib的方法*/
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
 
#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN]={0};     //接收缓冲,最大USART_REC_LEN个字节.
char USART_RX_BUF3[USART_REC_LEN]={0};
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	  
  
void uart_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART1, ENABLE);                    //使能串口1 

}

void uart2_init(u32 My_BaudRate)
{
	GPIO_InitTypeDef GPIO_InitStrue;
	USART_InitTypeDef USART_InitStrue;
	
	// 外设使能时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
//	USART_DeInit(USART2);  //复位串口2 -> 可以没有
	
	// 初始化 串口对应IO口  TX-PA2  RX-PA3
	GPIO_InitStrue.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStrue.GPIO_Pin=GPIO_Pin_2;
	GPIO_InitStrue.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStrue);
	
	// 初始化 串口模式状态
	USART_InitStrue.USART_BaudRate=My_BaudRate; // 波特率
	USART_InitStrue.USART_HardwareFlowControl=USART_HardwareFlowControl_None; // 硬件流控制
	USART_InitStrue.USART_Mode=USART_Mode_Tx; // 发送 模式
	USART_InitStrue.USART_Parity=USART_Parity_No; // 没有奇偶校验
	USART_InitStrue.USART_StopBits=USART_StopBits_1; // 一位停止位
	USART_InitStrue.USART_WordLength=USART_WordLength_8b; // 每次发送数据宽度为8位
	USART_Init(USART2,&USART_InitStrue);
	
	USART_Cmd(USART2,ENABLE);//使能串口
}

void uart3_init(u32 bound)
{  
 
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);	// GPIOB时钟      这里暂时搞成了PC10,11引脚！重定义功能！！！！！！！！
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE); //串口3时钟使能
 
   
// 	USART_DeInit(USART3);  //复位串口3
		 //USART3_TX   PB10
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB10
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOC, &GPIO_InitStructure); //初始化PB10
   
    //USART3_RX	  PB11
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOC, &GPIO_InitStructure);  //初始化PB11
    
	 GPIO_PinRemapConfig(GPIO_PartialRemap_USART3,ENABLE);   //开启重映射！！！！！！！！！！！！！！！
     
	USART_InitStructure.USART_BaudRate = bound;//波特率一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  
	USART_Init(USART3, &USART_InitStructure); //初始化串口	3
  
 
	USART_Cmd(USART3, ENABLE);                    //使能串口 
	
	//使能接收中断
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启中断   
	
	//设置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
}

//void USART1_IRQHandler(void)                	//串口1中断服务程序
//	{
//	u8 Res;
//#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
//	OSIntEnter();    
//#endif
//	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
//		{
//		Res =USART_ReceiveData(USART1);	//读取接收到的数据
//		
//		if((USART_RX_STA&0x8000)==0)//接收未完成
//			{
//			if(USART_RX_STA&0x4000)//接收到了0x0d
//				{
//				if(Res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
//				else USART_RX_STA|=0x8000;	//接收完成了 
//				}
//			else //还没收到0X0D
//				{	
//				if(Res==0x0d)USART_RX_STA|=0x4000;
//				else
//					{
//					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
//					USART_RX_STA++;
//					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
//					}		 
//				}
//			}   		 
//     } 
//#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
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

void USART1_IRQHandler(void)					//DMA+串口空闲中断
{
	if(USART_GetITStatus(USART1, USART_IT_IDLE) == SET)     //若接收完一轮语音消息
	{
		countA = USART_ReceiveData(USART1);                 //？？？？？？？？？？？？？？
		USART_ClearITPendingBit(USART1,USART_IT_IDLE);      //清除串口空闲中断标志位
		if(Receive_Buff[0]=='+')DMASign = SET;              //Receive_Buff[0]处特殊符号判断是否接收到语音消息，如果是，DMASign置1
		DMA_Cmd(DMA1_Channel5, DISABLE);					//重启一次DMA，保证每次都从BUFF的第一位开始
        
		DMA_ReStart((uint32_t)Receive_Buff);                //一直从DR寄存器读取256次相同的字节到Receive_Buff数组？？？？
        
	}else if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)	//数据由WiFi传给串口1，再把数据装入Receive_Buff数组
	{
		Receive_Buff[countA] = USART_ReceiveData(USART1);
		countA++;
	}
}
  
void USART3_IRQHandler(void)                	//串口3中断服务程序
{
	u8 Res;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) 
	{
		Res =USART_ReceiveData(USART3);//(USART1->DR);	//读取接收到的数据
	
	if(Res == '$')
	{
		point1 = 0;	
	}
		

	  USART_RX_BUF3[point1++] = Res;

	if(USART_RX_BUF3[0] == '$' && USART_RX_BUF3[4] == 'M' && USART_RX_BUF3[5] == 'C')			//确定是否收到"GPRMC/GNRMC"这一帧数据
	{
		if(Res == '\n')									   
		{
			memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);      //清空
			memcpy(Save_Data.GPS_Buffer, USART_RX_BUF3, point1); 	//保存数据
			Save_Data.isGetData = true;
			point1 = 0;
			memset(USART_RX_BUF3, 0, USART_REC_LEN);      //清空				
		}	
				
	}
	
    if(USART_RX_BUF3[0] == '$' && USART_RX_BUF3[1] == 'G' && USART_RX_BUF3[2] == 'P' && USART_RX_BUF3[3] == 'G' && USART_RX_BUF3[9] == '1')			//注意USART_RX_BUF[9] == '1'。为什么要这么写，因为如果只是像获取经纬度方式那样写，main函数里面数据更新速度比不上串口中断服务函数，获取的信息可能来自GPGSV的第二条或第三条，而非第一条
	{
		if(Res == '\n')									   
		{
			memset(Save_Data1.GPS_Buffer1, 0, GPS_Buffer_Length);      //清空
			memcpy(Save_Data1.GPS_Buffer1, USART_RX_BUF3, point1); 	//保存数据
			Save_Data1.isGetData1 = true;
			point1 = 0;
			memset(USART_RX_BUF3, 0, USART_REC_LEN);      //清空				
		}	
				
	}
    
	if(point1 >= USART_REC_LEN)
	{
		point1 = USART_REC_LEN;
	}	
		
   } 
}
void Serial_SendString(char Strings[],FlagStatus LineBreak) //LineBreak即是否添加换行
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
/*注意下面两个函数调整为了串口3*/
u8 Hand(char *a)                   // 串口命令识别函数
{ 
    if(strstr(USART_RX_BUF3,a)!=NULL)
	    return 1;
	else
		return 0;
}

void CLR_Buf(void)                           // 串口缓存清理
{
	memset(USART_RX_BUF3, 0, USART_REC_LEN);      //清空
    point1 = 0;                    
}

void clrStruct()
{
	Save_Data.isGetData = false;
	Save_Data.isParseData = false;
	Save_Data.isUsefull = false;
	memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);      //清空
	memset(Save_Data.UTCTime, 0, UTCTime_Length);
	memset(Save_Data.latitude, 0, latitude_Length);
	memset(Save_Data.N_S, 0, N_S_Length);
	memset(Save_Data.longitude, 0, longitude_Length);
	memset(Save_Data.E_W, 0, E_W_Length);
	
}
#endif	

