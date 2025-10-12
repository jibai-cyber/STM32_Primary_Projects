#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
//#include "lcd.h"
//#include "usart.h"	 
#include "adc.h"
#include "oled1.h"
 
/************************************************
 ALIENTEK��ӢSTM32������ʵ��17
 ADC ʵ��   
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

 u16 adcx,a[90],b[90],cur_max;  
 u8 cnta=0,cntb=0,flag_ab,flag_cntb,distance[2],flag_dis=0,dis=0;
 int main(void)
 {	 
    u16 adcx1;
    u8 i,adcy,adcy_pre;
	float temp,fre;
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
//	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
 	LED_Init();			     //LED�˿ڳ�ʼ��
//	LCD_Init();	
    OLED_Init();     
 	

	//��ʾ��ʾ��Ϣ
	OLED_ShowString(0,0,"3.0V",16);
    OLED_ShowString(0,16,"2.5V",16);
    OLED_ShowString(0,32,"2.0V",16);
    OLED_Clear_part(37,0,37,51,1);
    OLED_Clear_part(35,0,37,0,1);
    OLED_Clear_part(35,26,37,26,1);
    OLED_Clear_part(35,51,37,51,1);
    OLED_ShowString(15,52,"Vm=0.000 f=10.00KH",12);
     
//    OLED_ShowString(0,0,"ADC_VAL:",16);
//    OLED_ShowString(0,16,"ADC_VOL:0.000V",16);
    OLED_Refresh_Gram();
    Adc_Init();		  		//ADC��ʼ��
//	LCD_ShowString(60,130,200,16,16,"ADC_CH0_VAL:");	      
//	LCD_ShowString(60,150,200,16,16,"ADC_CH0_VOL:0.000V");	       
	while(1)
	{
        //LED0 = 0;
        if(flag_ab)
        {
            
            OLED_Clear_part(38,0,127,51,0);     //���һ������ͼ��
            for(i=0;i<90;i++)   //ÿ�β������7/6us�����������90������������105us
            {
                if(a[i]>=3724)
                {
                    OLED_DrawPoint(37+i,0,1);
                    if(i>0) OLED_Clear_part(37+i,0,37+i,51-adcy_pre,1);   //�൱��һ������ǰ������ĺ�����ʹͼ����ȥ��������           ������˵��ô��ʾ��ô����ԭ���������������OLED_Refresh_Gram����ȥ����
                    adcy_pre = 51;
                }
                else if(a[i]<=2482)
                {
                    OLED_DrawPoint(37+i,51,1);
                    if(i>0) OLED_Clear_part(37+i,51,37+i,51-adcy_pre,1);   //�൱��һ������ǰ������ĺ�����ʹͼ����ȥ��������
                    adcy_pre = 0;
                }
                else
                {
                    temp = (float)(a[i]-2482)/23.58 + 0.5;    //�������������2V��׼�߶����OLED��������������������
                    adcy = (u8)temp;
                    if(i>0) 
                    {
                        if(52-adcy<=51-adcy_pre) OLED_Clear_part(37+i,52-adcy,37+i,51-adcy_pre,1);   //�൱��һ������ǰ������ĺ�����ʹͼ����ȥ��������
                        else OLED_Clear_part(37+i,51-adcy_pre,37+i,52-adcy,1);
                    }
                       
                    adcy_pre = adcy;
                    OLED_DrawPoint(37+i,51-adcy,1);     //��ԭ����������ϴ�ӡ��ƫ�Ƶ������
                }
                
            }
            
            adcx1 = cur_max;   //����һ�ε�ǰֵ����ѭ����ɺ��ٸ��£����ⱻ�жϴ��
            temp=(float)adcx1*(3.3/4096);
            adcx1=temp;
            OLED_ShowNum(33,52,adcx1,1,12,1);
            temp-=adcx1;
            temp*=1000;
            adcx1=temp;
            OLED_ShowNum(45,52,adcx1,3,12,1);
            
            
            fre = (float)(1.0/(dis*(13.0/6)*2))*1000/0.77;     //��λΪKHz
            adcx1 = fre;
            OLED_ShowNum(81,52,adcx1,2,12,1);
            fre -= adcx1;
            fre*=100;
            adcx1 = fre;
            OLED_ShowNum(99,52,adcx1,2,12,1);
//            OLED_ShowNum(99,52,dis,2,12,1);
            
            OLED_Refresh_Gram();
            flag_ab = 0;
            flag_dis = 0;
            
            distance[0] = distance[1] = 0;
//            ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);//�����ж�
            
//            delay_ms(250);
//            LED0 = !LED0;
        }
        
//        adcx1 = adcx;   //����һ�ε�ǰֵ����ѭ����ɺ��ٸ��£����ⱻ�жϴ��
//        OLED_ShowNum(64,0,adcx1,4,16,1);
//        temp=(float)adcx1*(3.3/4096);
//        adcx1=temp;
//        OLED_ShowNum(64,16,adcx1,1,16,1);
//        temp-=adcx1;
//        temp*=1000;
//        OLED_ShowNum(80,16,temp,3,16,1);
//        OLED_Refresh_Gram();
        
        
//		adcx=Get_Adc_Average(ADC_Channel_1,10);
//		LCD_ShowxNum(156,130,adcx,4,16,0);//��ʾADC��ֵ
//		temp=(float)adcx*(3.3/4096);
//		adcx=temp;
//		LCD_ShowxNum(156,150,adcx,1,16,0);//��ʾ��ѹֵ
//		temp-=adcx;
//		temp*=1000;
//		LCD_ShowxNum(172,150,temp,3,16,0X80);
//		LED0=!LED0;
//		delay_ms(250);	
	}
 }

