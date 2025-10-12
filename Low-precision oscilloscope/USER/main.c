#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
//#include "lcd.h"
//#include "usart.h"	 
#include "adc.h"
#include "oled1.h"
 
/************************************************
 ALIENTEK精英STM32开发板实验17
 ADC 实验   
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

 u16 adcx,a[90],b[90],cur_max;  
 u8 cnta=0,cntb=0,flag_ab,flag_cntb,distance[2],flag_dis=0,dis=0;
 int main(void)
 {	 
    u16 adcx1;
    u8 i,adcy,adcy_pre;
	float temp,fre;
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
//	uart_init(115200);	 	//串口初始化为115200
 	LED_Init();			     //LED端口初始化
//	LCD_Init();	
    OLED_Init();     
 	

	//显示提示信息
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
    Adc_Init();		  		//ADC初始化
//	LCD_ShowString(60,130,200,16,16,"ADC_CH0_VAL:");	      
//	LCD_ShowString(60,150,200,16,16,"ADC_CH0_VOL:0.000V");	       
	while(1)
	{
        //LED0 = 0;
        if(flag_ab)
        {
            
            OLED_Clear_part(38,0,127,51,0);     //清除一次坐标图像
            for(i=0;i<90;i++)   //每次采样间隔7/6us，所以这里的90格代表的量程是105us
            {
                if(a[i]>=3724)
                {
                    OLED_DrawPoint(37+i,0,1);
                    if(i>0) OLED_Clear_part(37+i,0,37+i,51-adcy_pre,1);   //相当于一个连接前后两点的函数，使图像看上去更加连贯           靠！我说怎么显示这么慢，原来是这个函数最后的OLED_Refresh_Gram忘记去掉了
                    adcy_pre = 51;
                }
                else if(a[i]<=2482)
                {
                    OLED_DrawPoint(37+i,51,1);
                    if(i>0) OLED_Clear_part(37+i,51,37+i,51-adcy_pre,1);   //相当于一个连接前后两点的函数，使图像看上去更加连贯
                    adcy_pre = 0;
                }
                else
                {
                    temp = (float)(a[i]-2482)/23.58 + 0.5;    //这里计算出相对于2V基准线多出的OLED格子数，并且四舍五入
                    adcy = (u8)temp;
                    if(i>0) 
                    {
                        if(52-adcy<=51-adcy_pre) OLED_Clear_part(37+i,52-adcy,37+i,51-adcy_pre,1);   //相当于一个连接前后两点的函数，使图像看上去更加连贯
                        else OLED_Clear_part(37+i,51-adcy_pre,37+i,52-adcy,1);
                    }
                       
                    adcy_pre = adcy;
                    OLED_DrawPoint(37+i,51-adcy,1);     //在原点坐标基础上打印出偏移点的坐标
                }
                
            }
            
            adcx1 = cur_max;   //保留一次当前值，主循环完成后再更新，以免被中断打断
            temp=(float)adcx1*(3.3/4096);
            adcx1=temp;
            OLED_ShowNum(33,52,adcx1,1,12,1);
            temp-=adcx1;
            temp*=1000;
            adcx1=temp;
            OLED_ShowNum(45,52,adcx1,3,12,1);
            
            
            fre = (float)(1.0/(dis*(13.0/6)*2))*1000/0.77;     //单位为KHz
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
//            ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);//开启中断
            
//            delay_ms(250);
//            LED0 = !LED0;
        }
        
//        adcx1 = adcx;   //保留一次当前值，主循环完成后再更新，以免被中断打断
//        OLED_ShowNum(64,0,adcx1,4,16,1);
//        temp=(float)adcx1*(3.3/4096);
//        adcx1=temp;
//        OLED_ShowNum(64,16,adcx1,1,16,1);
//        temp-=adcx1;
//        temp*=1000;
//        OLED_ShowNum(80,16,temp,3,16,1);
//        OLED_Refresh_Gram();
        
        
//		adcx=Get_Adc_Average(ADC_Channel_1,10);
//		LCD_ShowxNum(156,130,adcx,4,16,0);//显示ADC的值
//		temp=(float)adcx*(3.3/4096);
//		adcx=temp;
//		LCD_ShowxNum(156,150,adcx,1,16,0);//显示电压值
//		temp-=adcx;
//		temp*=1000;
//		LCD_ShowxNum(172,150,temp,3,16,0X80);
//		LED0=!LED0;
//		delay_ms(250);	
	}
 }

