#include "stm32f10x.h"
void DMA_Start(uint32_t ADRdes)
{
    DMA_InitTypeDef DMA_InitStruct;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);												//挂设在AHB时钟线
	
	
	DMA_InitStruct.DMA_BufferSize = 256;																		//每一轮传输次数
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC; 												//方向  Periphral为目标 SRC就是Memory为目标
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;																//1软件控制  0硬件控制
	DMA_InitStruct.DMA_MemoryBaseAddr = ADRdes; 														//Memory寄存器
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;						//需要转存的每个数据大小
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;										//自增
	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;															//循环
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;					//Peripheral寄存器
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;		//需要转存的每个数据大小
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;						//不自增
	DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;											//配置转运优先级
	
	DMA_Init(DMA1_Channel5, &DMA_InitStruct);
	
	USART_Cmd(USART1, DISABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);																	//关闭接受中断
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);																	//打开空闲中断
	
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);																	//开启DMA转运
	
	USART_Cmd(USART1, ENABLE);
	DMA_Cmd(DMA1_Channel5, ENABLE);
}
void DMA_ReStart(uint32_t ADRdes)
{
	DMA_InitTypeDef DMA_InitStruct;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);												//挂设在AHB时钟线
	
	
	DMA_InitStruct.DMA_BufferSize = 256;																		  //每一轮传输次数
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC; 												//方向  Periphral为目标 SRC就是Memory为目标
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;																//1软件控制  0硬件控制
	DMA_InitStruct.DMA_MemoryBaseAddr = ADRdes; 														//Memory寄存器
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;						//需要转存的每个数据大小
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;										//自增
	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;															//不循环
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;					//Peripheral寄存器
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;		//需要转存的每个数据大小
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;						//不自增
	DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;											//配置转运优先级
	
	DMA_Init(DMA1_Channel5, &DMA_InitStruct);
	
	DMA_Cmd(DMA1_Channel5, ENABLE);
}
