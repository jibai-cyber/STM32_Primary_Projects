#ifndef __OLED1_H
#define __OLED1_H
#include "stm32f10x.h"
#include "sys.h"

/*��������*/
#define OLED_W_SCL(x)  GPIO_WriteBit(GPIOB, GPIO_Pin_6, (BitAction)(x))
#define OLED_W_SDA(x)  GPIO_WriteBit(GPIOB, GPIO_Pin_7, (BitAction)(x))

void OLED_Init(void);//OLED��ʼ��
void OLED_Clear(void);//����
void OLED_Refresh_Gram(void);//�����Դ浽OLED
void OLED_WriteData(uint8_t Data);//OLEDд����
void OLED_WriteCommand(uint8_t Command);//OLEDд����
void OLED_DrawPoint(u8 x,u8 y,u8 t);//����
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size,u8 mode);//oled��ʾ�ַ�
void OLED_ShowString(u8 x,u8 y,const u8 *p,u8 size);//oled��ʾ�ַ���
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size,u8 mode);//oled��ʾ����  ע�⣬�����и��ģ���������һ������mode��
void OLED_Show_ChineseChar(u8 x,u8 y,u8 num,u8 size,u8 mode);
void OLED_Show_ChineseString(u8 x,u8 y,const u8 *p,u8 size);
void OLED_ShowGraph(u8 x,u8 y,u8 Graph_num,u8 ysize,u16 size);
void OLED_Clear_part(u8 x0,u8 y0,u8 x1,u8 y1,u8 mode);
void OLED_Opposite(u8 x0,u8 y0,u8 x1,u8 y1);
u8 OLED_Judge(u8 x,u8 y);
u32 mypow(u8 m,u8 n);//m^n����
void OLED_ShowChinese(u8 x,u8 y,u8 num,u8 size,u8 mode);//oled��ʾ����
#endif



