#include "led.h"
#include "delay.h"
#include "key.h"
#include "oled1.h"
#include "sys.h"
#include "usart.h"
#include "mpu6050.h"
#include "usmart.h"   
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h" 
#include <string.h>
#include "timer.h"
#include "beep.h"
#include "DMA.h"
#include "ESP8266.h"
#include "SYN6288.h"
 

/*
注意：可能mpu6050初始化时间较长，如果显示wait就等待一会，如果时间过长就检查接线或者重启
*/

/*
接线方面

加速度模块
SCL->PB10
SDA->PB11


gps模块
1.接收原数据模式
gps_RXD->TXD
gps_TXD->RXD
2.接收解析后数据模式
gps_TXD->PC11
PC10->RXD
3.不需要用串口助手检查数据，最终直接使用
gps_TXD->PC11


WiFi模块
串口1


语音模块
串口2


OLED
SCL->PB6
SDA->PB7
*/

#define ABS(X)  (((X)>0)?(X):-(X))

u8 Chinese_str1[]={3,4,5,0};    //中文字符串，末尾加0，详见OLED_Show_ChineseString函数
u8 Chinese_str2[]={3,4,6,0};
u8 Chinese_str3[]={7,8,0};
u8 Chinese_str4[]={11,12,0};
u8 Chinese_str5[]={13,14,0};
u8 Chinese_str6[]={15,16,17,0};
u8 Chinese_str7[]={20,21,22,23,0};
u8 Chinese_str8[]={37,38,39,40,0};
u8 Chinese_str9[]={30,31,32,33,0};
struct Menu     //菜单结构体
{
    u8 pre; //上一个界面索引号
    u8 cur; //当前界面索引号，目前无实际作用
    u8 next;    //下一个界面索引号
    u8 inside;  //下一级界面索引号
    u8 out; //退出界面索引号，按键数量原因目前还没用到
    void (*operation)(void);    //索引号下的功能函数
};
  
u8 index_num=0,flag_opposite=0,flag_phone1_show=0,flag_home1_show=0,flag_choice_pre,member_called,light_level=0;
/*函数名声明*/
void home(void);
void phone(void);
void set_member(void);
void weather(void);

void home1(void);
void phone1(void);
void set_member1(void);
void weather1(void);

void choice(void);


u8 history_cnt=0,history_flag=0,history_exceed=0;
u8 history_member[4],history_time[4][6];    //history_time记得末尾补0
u8 loudness=3;
u8 useful_flag=0;
u8 pwn_num=0;
u8 t=0,mode_report=1;			//默认开启上报
u8 key,flag_report,report=1,gps_report=1;
u8 timer_flag=0,first_flag=1,time_cnt=0;
int fall_flag=0;
float pitch,roll,yaw; 		//欧拉角
short aacx,aacy,aacz;		//加速度传感器原始数据
float aacx_pre,aacx_cur,aacy_pre,aacy_cur,aacz_pre,aacz_cur;
short gyrox,gyroy,gyroz;	//陀螺仪原始数据
short temp,cnt_wait=0;
short jueduizhi(short num);

void LED_light(u8 light);

void errorLog(int num);
void parseGpsBuffer(void);
void printGpsBuffer(void);
char Time[9];  //时间字符串数组
char Latitude[12];    //纬度字符串数组
char Longitude[13];   //经度字符串数组
char Weixingnum[3];  //卫星数量字符串数组
char Xinhao[3]; //信号强度字符串数组

extern char Receive_Buff[256];
extern char broadcast[128];
extern FlagStatus DMASign;

char number1[] = "11111";
char number2[] = "22222";
char number3[] = "33333";

/*结构体数组，最后的函数对应上面声明过的函数*/
struct Menu table[]={
    {3,0,1,4,0,(*home)},    //一级
    {0,1,2,5,0,(*phone)},
    {1,2,3,6,0,(*set_member)},
    {2,3,0,7,0,(*weather)},
    
    {0,4,0,8,0,(*home1)},   //二级
    {0,5,0,8,1,(*phone1)}, 
    {0,6,0,8,2,(*set_member1)},
    {0,7,0,8,3,(*weather1)},
    
    {0,8,0,0,0,(*choice)}   //三级
};

//     OLED_Clear_part(9,1,26,20,0);    //信号依次增强
//     OLED_Clear_part(15,1,26,20,0);
//     OLED_Clear_part(21,1,26,20,0);

/*按键扫描函数，判定实行哪一个函数*/
void key_set(void)
{
    if(index_num!=8 && index_num!=5 && index_num!=4)    //注意！！！！！这里的索引号不能是需要使用按键实现其他功能的界面的索引号
        switch(KEY_Scan(0))
        {
            case WKUP_PRES:index_num = table[index_num].pre; OLED_Clear(); break;   //按下按键后索引号变为当前结构体的上一层索引号
            case KEY0_PRES:index_num = table[index_num].next; OLED_Clear(); break;
            case KEY1_PRES:
                index_num = table[index_num].inside;OLED_Clear();
                switch(index_num)
                {
                    case 5: flag_phone1_show = 1;break; //这里的flag都是用于显示的一些操作
                    case 4: flag_home1_show = 1;break;
                    default:break;
                }                    
                break;
                
            default: break;
        }
   

    
    table[index_num].operation();   //实现当下索引号的函数功能
}

void home()
{
     OLED_ShowGraph(0,0,2,64,1024);
     if(Time[0]=='\0')
     OLED_ShowString(30,0,"88:88:88",16);
     else OLED_ShowString(30,0,Time,16);
     OLED_ShowString(100,0,"sgn",12);
     OLED_ShowString(100,12,Xinhao,12);
     OLED_ShowGraph(48,18,3,32,128);
     OLED_ShowString(0,47,"0.00 0.00 Num:00",16);
     if(flag_opposite) OLED_Opposite(0,0,127,63);   //反差色判断
     else OLED_Refresh_Gram();
}

void phone()
{
     OLED_ShowGraph(0,0,2,64,1024);
     if(Time[0]=='\0')
     OLED_ShowString(30,0,"88:88:88",16);
     else OLED_ShowString(30,0,Time,16);
     OLED_ShowString(100,0,"sgn",12);
     OLED_ShowString(100,12,Xinhao,12);
     OLED_ShowGraph(44,18,1,32,156);
     OLED_ShowString(0,47,"0.00 0.00 Num:00",16);
     if(flag_opposite) OLED_Opposite(0,0,127,63);
     else OLED_Refresh_Gram();
}
void set_member()
{
     OLED_ShowGraph(0,0,2,64,1024);
     if(Time[0]=='\0')
     OLED_ShowString(30,0,"88:88:88",16);
     else OLED_ShowString(30,0,Time,16);
     OLED_ShowString(100,0,"sgn",12);
     OLED_ShowString(100,12,Xinhao,12);
     OLED_ShowGraph(44,18,4,32,156);
     OLED_ShowString(0,47,"0.00 0.00 Num:00",16);
     if(flag_opposite) OLED_Opposite(0,0,127,63);
     else OLED_Refresh_Gram();
}

void weather()
{
     OLED_ShowGraph(0,0,2,64,1024);
     if(Time[0]=='\0')
     OLED_ShowString(30,0,"88:88:88",16);
     else OLED_ShowString(30,0,Time,16);
     OLED_ShowString(100,0,"sgn",12);
     OLED_ShowString(100,12,Xinhao,12);
     OLED_ShowGraph(41,18,5,32,180);
     OLED_ShowString(0,47,"0.00 0.00 Num:00",16);
     if(flag_opposite) OLED_Opposite(0,0,127,63);
     else OLED_Refresh_Gram();
}

void choice()
{
    u8 i;
    switch(flag_choice_pre) //这里的三级函数只写了一个，所以二级菜单到三级菜单的时候要把其索引号赋值给flag_choice_pre变量
    {
        case 5:
            OLED_Show_ChineseChar(28,20,0,24,1);
            OLED_Show_ChineseChar(52,20,1,24,1);
            OLED_Show_ChineseChar(76,20,2,24,1);
            OLED_Show_ChineseChar(28,46,9,16,1);
            OLED_Show_ChineseChar(76,46,10,16,1);
            OLED_Refresh_Gram();
           switch(KEY_Scan(0))
        {
//            case WKUP_PRES:index_num = table[index_num].pre; OLED_Clear(); break;
            case KEY1_PRES:
                index_num = table[index_num].next; 
                if(member_called == 0)
                {
                    ESP8266_recall(number1);
                    if(history_exceed)
                    {
                        for(i=3;i>0;i--)
                        {
                            history_member[i] = history_member[i-1];
                            strcpy(history_time[i],history_time[i-1]);
                        }
                        for(i=0;i<5;i++)
                            history_time[0][i] = Time[i];
                        history_member[0] = member_called+1;
                    }
                    else
                    {
                        for(i=0;i<5;i++)
                            history_time[history_cnt][i] = Time[i];
                        history_member[history_cnt] = member_called+1;
                        history_cnt++;
                    }
                    
                    if(history_cnt==4)
                    {
                        history_cnt = 0;
                        history_exceed = 1;
                    }
                    history_flag = 1;
                }
                else if(member_called == 1)
                {
                    ESP8266_recall(number2);
                    
                   if(history_exceed)
                    {
                        for(i=3;i>0;i--)
                        {
                            history_member[i] = history_member[i-1];
                            strcpy(history_time[i],history_time[i-1]);
                        }
                        for(i=0;i<5;i++)
                            history_time[0][i] = Time[i];
                        history_member[0] = member_called+1;
                    }
                    else
                    {
                        for(i=0;i<5;i++)
                            history_time[history_cnt][i] = Time[i];
                        history_member[history_cnt] = member_called+1;
                        history_cnt++;
                    }
                    
                    if(history_cnt==4)
                    {
                        history_cnt = 0;
                        history_exceed = 1;
                    }
                    history_flag = 1;
                }
                else if(member_called == 2)
                {
                    ESP8266_recall(number3);
                    
                    if(history_exceed)
                    {
                        for(i=3;i>0;i--)
                        {
                            history_member[i] = history_member[i-1];
                            strcpy(history_time[i],history_time[i-1]);
                        }
                        for(i=0;i<5;i++)
                            history_time[0][i] = Time[i];
                        history_member[0] = member_called+1;
                    }
                    else
                    {
                        for(i=0;i<5;i++)
                            history_time[history_cnt][i] = Time[i];
                        history_member[history_cnt] = member_called+1;
                        history_cnt++;
                    }
                    
                    if(history_cnt==3)
                    {
                        history_cnt = 0;
                        history_exceed = 1;
                    }
                    history_flag = 1;
                }
                OLED_Clear();
                break;
            
            case KEY0_PRES:index_num = flag_choice_pre; OLED_Clear(); break;
            default: break;
        }
            break;
        
        default: break;
    }
        
}

void home1()
{
    static u8 white_position;   //用于选中白色区域
    u8 judge[4]={15,31,47,63};  //用来表示四个区域的区间，方便调用
    if(flag_home1_show)    //保证文本内容只显示一次,注意这里的flag有所区别
    {
        flag_home1_show=0;  //别忘置0
        white_position=0;
        OLED_ShowString(0,0,"1.",16);
        OLED_Show_ChineseString(16,0,Chinese_str4,16);
        OLED_ShowString(0,16,"2.",16);
        OLED_Show_ChineseString(16,16,Chinese_str5,16);
        OLED_ShowString(0,32,"3.",16);
        OLED_Show_ChineseString(16,32,Chinese_str6,16);
        OLED_Show_ChineseString(16,48,Chinese_str7,16);
        OLED_ShowNum(110,0,light_level,1,16,!OLED_Judge(127,judge[0])); //通过判断所在区域一个像素点的颜色决定该字符颜色
        OLED_ShowNum(110,16,loudness,1,16,!OLED_Judge(127,judge[1]));
        OLED_Show_ChineseChar(110,32,flag_opposite+18,16,!OLED_Judge(127,judge[2]));
        OLED_Refresh_Gram();
    }
    
    switch(KEY_Scan(0))
    {
        case WKUP_PRES:if(white_position==0) white_position=3; else white_position--; break;
        case KEY0_PRES:if(white_position==3) white_position=0; else white_position++; break;
        case KEY1_PRES:
            switch(white_position)
            {
                case 0:light_level++; light_level%=4; break;
                case 3: index_num = table[index_num].next;break;
                case 2: flag_opposite = !flag_opposite; break;
                case 1:loudness++; loudness%=4; break;
                default: break;
            }
            break;
        default: break;
    }
    
    
    switch(white_position)  //选中的白色区域将其反转
    {
        case 0:
            if(!OLED_Judge(127,judge[0]))
                OLED_Opposite(0,0,127,judge[0]);
            if(OLED_Judge(127,judge[1]))
               OLED_Opposite(0,16,127,judge[1]);
            if(OLED_Judge(127,judge[3]))
               OLED_Opposite(0,48,127,judge[3]);
            break;
        case 1:
            if(!OLED_Judge(127,judge[1]))
                OLED_Opposite(0,16,127,judge[1]);
            if(OLED_Judge(127,judge[2]))
               OLED_Opposite(0,32,127,judge[2]);
            if(OLED_Judge(127,judge[0]))
               OLED_Opposite(0,0,127,judge[0]);
            break;
        case 2:
            if(!OLED_Judge(127,judge[2]))
                OLED_Opposite(0,32,127,judge[2]);
            if(OLED_Judge(127,judge[3]))
               OLED_Opposite(0,48,127,judge[3]);
            if(OLED_Judge(127,judge[1]))
               OLED_Opposite(0,16,127,judge[1]);
            break;
        case 3:
            if(!OLED_Judge(127,judge[3]))
                OLED_Opposite(0,48,127,judge[3]);
            if(OLED_Judge(127,judge[0]))
               OLED_Opposite(0,0,127,judge[0]);
            if(OLED_Judge(127,judge[2]))
               OLED_Opposite(0,32,127,judge[2]);
            break;
        default:
             break;
    }
    
    OLED_ShowNum(110,0,light_level,1,16,!OLED_Judge(127,judge[0])); //反转区域颜色后，再通过判断决定该字符颜色
    OLED_ShowNum(110,16,loudness,1,16,!OLED_Judge(127,judge[1]));
    OLED_Show_ChineseChar(110,32,flag_opposite+18,16,!OLED_Judge(127,judge[2]));
    OLED_Refresh_Gram();
}

void set_member1()
{
    u8 i;
    u8 place[4]={0,16,32,48};
    static u8 flag;
    switch(KEY_Scan(0))
    {
        case KEY1_PRES:
            index_num = table[index_num].out;
            OLED_Clear();
            break;
        default: break;
    }
        
    
    if(!history_flag)
    {
        OLED_Show_ChineseString(32,24,Chinese_str8,16);
        OLED_Refresh_Gram();
        flag = 1;
    }
    else
    {
        if(flag)
        {
            flag = 0;
            OLED_Clear();
        }
        
        for(i=0;i<4;i++)
        switch(history_member[i])
        {
            case 1:
                OLED_Show_ChineseString(0,place[i],Chinese_str1,16);
                OLED_ShowString(87,place[i],history_time[i],16);
                OLED_Refresh_Gram();
                break;
            case 2:
                OLED_Show_ChineseString(0,place[i],Chinese_str2,16);
                OLED_ShowString(87,place[i],history_time[i],16);
                OLED_Refresh_Gram();
                break;
            case 3:
                OLED_Show_ChineseString(0,place[i],Chinese_str3,16);
                OLED_ShowString(87,place[i],history_time[i],16);
                OLED_Refresh_Gram();
                break;
            default:
                break;
        }
    }
}

void weather1()
{
    u8 line[4]={0,16,32,48};
    //判断是否反差色，是否有gps解析,半角只占8格
    switch(KEY_Scan(0))
    {
        case KEY1_PRES:
            index_num = table[index_num].out;
            OLED_Clear();
            break;
        default: break;
    }
    if(useful_flag)
    {
        if(Save_Data.N_S[0] == 'N')
            OLED_Show_ChineseChar(0,line[0],27,16,1);
        else if(Save_Data.N_S[0] == 'S')
            OLED_Show_ChineseChar(0,line[0],26,16,1);
        OLED_Show_ChineseChar(16,line[0],29,16,1);
        OLED_ShowChar(32,line[0],Save_Data.latitude[0],16,1);
        OLED_ShowChar(40,line[0],Save_Data.latitude[1],16,1);
        OLED_Show_ChineseChar(48,line[0],35,16,1);
        OLED_ShowChar(64,line[0],Save_Data.latitude[2],16,1);
        OLED_ShowChar(72,line[0],Save_Data.latitude[3],16,1);
        OLED_Show_ChineseChar(80,line[0],36,16,1);

        
        if(Save_Data.E_W[0] == 'W')
            OLED_Show_ChineseChar(0,line[1],25,16,1);
        else if(Save_Data.E_W[0] == 'E')
            OLED_Show_ChineseChar(0,line[1],24,16,1);
        OLED_Show_ChineseChar(16,line[1],28,16,1);
        OLED_ShowChar(32,line[1],Save_Data.longitude[0],16,1);
        OLED_ShowChar(40,line[1],Save_Data.longitude[1],16,1);
        OLED_ShowChar(48,line[1],Save_Data.longitude[2],16,1);
        OLED_Show_ChineseChar(48+8,line[1],35,16,1);
        OLED_ShowChar(64+8,line[1],Save_Data.longitude[3],16,1);
        OLED_ShowChar(72+8,line[1],Save_Data.longitude[4],16,1);
        OLED_Show_ChineseChar(80+8,line[1],36,16,1);
        
        OLED_Show_ChineseString(0,line[2],Chinese_str9,16);
        OLED_ShowChar(64,line[2],':',16,1);
        OLED_ShowString(72,line[2],Weixingnum,16);
        
        
        OLED_Refresh_Gram();
    }
    
    else
    {
        OLED_ShowString(0,0,"gps is not avb",16);
        OLED_Refresh_Gram();
    }
    
}

void phone1()
{
    static u8 white_position;
    u8 judge[4]={15,31,47,63};
    if(flag_phone1_show)    //保证文本内容只显示一次
    {
        flag_phone1_show=0;
        white_position=0;
        OLED_ShowString(0,0,"1.",16);
        OLED_Show_ChineseString(16,0,Chinese_str1,16);
        OLED_ShowString(0,16,"2.",16);
        OLED_Show_ChineseString(16,16,Chinese_str2,16);
        OLED_ShowString(0,32,"3.",16);
        OLED_Show_ChineseString(16,32,Chinese_str3,16);
        OLED_Show_ChineseString(16,48,Chinese_str7,16);
        OLED_Refresh_Gram();
    }
    
    switch(KEY_Scan(0))
    {
        case WKUP_PRES:if(white_position==0) white_position=3; else white_position--; break;
        case KEY0_PRES:if(white_position==3) white_position=0; else white_position++; break;
        case KEY1_PRES:
            if(white_position==3) {index_num = table[index_num].out; break;}    //返回键直接退出
            else {flag_choice_pre = index_num; index_num = table[index_num].inside; OLED_Clear(); goto out;}    //out在本函数末尾处
        default: break;
    }
    
    switch(white_position)
    {
        case 0:
            if(!OLED_Judge(127,judge[0]))
                OLED_Opposite(0,0,127,judge[0]);
            if(OLED_Judge(127,judge[1]))
               OLED_Opposite(0,16,127,judge[1]);
            if(OLED_Judge(127,judge[3]))
               OLED_Opposite(0,48,127,judge[3]);
            break;
        case 1:
            if(!OLED_Judge(127,judge[1]))
                OLED_Opposite(0,16,127,judge[1]);
            if(OLED_Judge(127,judge[2]))
               OLED_Opposite(0,32,127,judge[2]);
            if(OLED_Judge(127,judge[0]))
               OLED_Opposite(0,0,127,judge[0]);
            break;
        case 2:
            if(!OLED_Judge(127,judge[2]))
                OLED_Opposite(0,32,127,judge[2]);
            if(OLED_Judge(127,judge[3]))
               OLED_Opposite(0,48,127,judge[3]);
            if(OLED_Judge(127,judge[1]))
               OLED_Opposite(0,16,127,judge[1]);
            break;
        case 3:
            if(!OLED_Judge(127,judge[3]))
                OLED_Opposite(0,48,127,judge[3]);
            if(OLED_Judge(127,judge[0]))
               OLED_Opposite(0,0,127,judge[0]);
            if(OLED_Judge(127,judge[2]))
               OLED_Opposite(0,32,127,judge[2]);
            break;
        default:
             break;
    }
    out:member_called = white_position;     //member_called记录被呼叫的联系人序号，便于区分
}

 int main(void)
 {	 
  char *buffA = broadcast;																		
	char *buffB = &Receive_Buff[34];														//指向"call"的第一个字符    
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	UART_init();	 	//串口初始化为9600
	delay_init();	//延时初始化 
	usmart_dev.init(72);		//初始化USMART
  OLED_Init();    //必须先初始化oled，否则显示不了wifi调试
  ESP8266_Setup();																						//ESP8266初始化(需要较长时间，预计15S)
	DMA_Start((uint32_t)Receive_Buff);
	LED_Init();		  			//初始化与LED连接的硬件接口
	KEY_Init();					//初始化按键
	MPU_Init();					//初始化MPU6050
  BEEP_Init();
  clrStruct();
//	while(mpu_dmp_init())   //如果数据还未初始化成功
// 	{
//        OLED_ShowString(10,20,"WAIT!",24);
//        OLED_Refresh_Gram();
//	}  
    OLED_Clear();
    
    home();
    
    TIM5_Init();   //每300ms发送一次数据
    TIM3_PWM_Init(1000,0);
 	while(1)
	{
//        LED_light(light);
        if(gps_report)   //3s解析并上传一次gps数据
        {
    //     gps_report = false;
           parseGpsBuffer();
           printGpsBuffer();
        }
        
        if(!DMASign)
        {
          if(gps_report)
          {
             gps_report = false;     //避免反复上传
             ESP8266_Sendcondition(Longitude,Latitude,fall_flag);  //DMASign用作是否接收完语音消息
          }
        }    
        else    //若接收完成，由串口接收到的数据放在Receive_Buff数组里（从第34个元素开始，至于为什么，可能是网络数据分析后如此），再由Receive_Buff数组转入broadcast数组，再进行播报
            {
                DMASign = RESET;
                while(*buffB != '"')*buffA++ = *buffB++;
                *buffA = '\0';  //数组末尾加0进行播报
                switch(loudness)
                {
                    case 0:TTSPlay('m',broadcast,0); break;
                    case 1:TTSPlay('m',broadcast,4); break;
                    case 2:TTSPlay('m',broadcast,9); break;
                    case 3:TTSPlay('m',broadcast,16); break;
                    default: break;
                }
//                TTSPlay('m',broadcast,4);   //4,9,16
                buffA = broadcast;	          
                buffB = &Receive_Buff[34];	
            }
        
//		key=KEY_Scan(0);    //第一步，获取键码
       
        
        /*灯光操作*/
//        light = light_level;
//        pwn_num+=1;
//        if(pwn_num>=3) pwn_num = 0;
//        LED_light(light);
        
        
        
//		if(key==KEY0_PRES)
//		{
//            /*每按一次按键换一种显示模式，默认是显示加速度，切换为显示偏移角*/
//			mode_report=!mode_report;   //0表示显示三个角度，1表示显示三个加速度
//              flag_report = 1;
//              if(mode_report && flag_report)
//              {
//                        flag_report = 0;
//                        OLED_Clear();
//                        OLED_ShowString(10,10," ax  :    . C",16);
//                        OLED_ShowString(10,26," ay  :    . C",16);
//                        OLED_ShowString(10,42," az  :    . C",16);
//                OLED_Refresh_Gram();
//               }else if(!mode_report && flag_report)
//               {
//                            flag_report = 0;
//                            OLED_Clear();
//                            OLED_ShowString(10,10,"Pitch:    . C",16);
//                            OLED_ShowString(10,26," Roll:    . C",16);
//                            OLED_ShowString(10,42," Yaw :    . C",16);
//                            OLED_Refresh_Gram();
//               }
//		}
//        
//        else if(key==KEY1_PRES)  //key1决定是否向串口发送数据，每按一次结果翻转一次，默认为上报
//        {
//			report = !report;
//        }
//        
//        else if(key==KEY14_PRES)
//		{
//			ESP8266_recall(number1);
//		}
//        
//        else if(key==KEY15_PRES)
//		{
//			ESP8266_recall(number2);
//		}
        
//		if(mpu_dmp_get_data(&pitch,&roll,&yaw)==0 && pitch||roll||yaw)  //如果接收到数据了
//		{ 
			temp=MPU_Get_Temperature();	//得到温度值
			MPU_Get_Accelerometer(&aacx,&aacy,&aacz);	//得到加速度传感器数据
			MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);	//得到陀螺仪数据
            if(first_flag)
            {
                first_flag = 0;
                aacx_cur = aacx_pre = aacx/16384.0f;
                aacy_cur = aacy_pre = aacy/16384.0f;
                aacz_cur = aacz_pre = aacz/16384.0f;
            }
            else
            {
                aacx_cur = aacx/16384.0f;
                aacy_cur = aacy/16384.0f;
                aacz_cur = aacz/16384.0f;
            }
            
            if(timer_flag)  //每300ms检测一次前后加速度差，如果大于一定值就会报警，具体的数值可以看185行的if，如果太容易报警了就适当调大一点
            {
                timer_flag = 0;
                if((ABS(aacx_cur - aacx_pre)>0.8f || ABS(aacy_cur - aacy_pre)>0.8f || ABS(aacz_cur - aacz_pre)>0.8f))    //默认加速度模块与单片机方向相反
                {
                    BEEP=1;
                    LED0=0;
                   // LED1=0;
                    fall_flag = 1;
                }
                else
                {
                    BEEP=0;
                    LED0=1;
                  //  LED1=1;
                }
                aacx_pre = aacx_cur;
                aacy_pre = aacy_cur;
                aacz_pre = aacz_cur;
//            }
            
		}
        
        key_set();
        
	} 	
}
 
//void LED_light(u8 light)
//{
//    if(pwn_num<=light && light!=0)
//        LED1 = 0;
//    else LED1 = 1;
//}


void TIM5_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)  //检查TIM5更新中断发生与否
		{
            TIM_ClearITPendingBit(TIM5, TIM_IT_Update  );  //清除TIMx更新中断标志
            
            pwn_num=light_level;               //四个灯光挡位，这里pwn_num加多一点是为了更快进到下一个pwn周期
            pwn_num%=4;
            TIM_SetCompare2(TIM3,pwn_num*250);
            
            if(report && (aacx||aacy||aacz||pitch||roll||yaw))
                timer_flag = true;     //每隔300ms判断加速度前后差距，如果变化量大于一定范围则警报
            
            time_cnt++;
            if(time_cnt==10)
            {
                time_cnt = 0;
//                LED0 = !LED0;   //这个用来表示定时器正常工作，不用可删
                gps_report = true;       //每3s汇报一次数据
            }
            
		}
}
 

void errorLog(int num)
{
	  printf("ERROR%d\r\n",num);  //num表示错误种类
}
/*解析NMEA报文*/
void parseGpsBuffer()
{
	char *subString,*subString1;    //用来标记子字符串起始位置（其实只用一个变量
	char *subStringNext,*subStringNext1;    //用来标记子字符串终止位置（其实只用一个
	char i = 0;
	if (Save_Data.isGetData)
	{
		Save_Data.isGetData = false;    //保证每次接收一次
		printf("**************\r\n");
		printf(Save_Data.GPS_Buffer);   //把该条原报文显示出来

		
		for (i = 0 ; i <= 6 ; i++)  //i的大小取决于最后一个数据排在第几个逗号
		{
			if (i == 0) //第一次进循环需先确定subString指针的位置
			{
				if ((subString = strstr(Save_Data.GPS_Buffer, ",")) == NULL)
					errorLog(1);	//解析错误
			}
			else
			{
				subString++;    //保证每次开始时subString在逗号后面一位
				if ((subStringNext = strstr(subString, ",")) != NULL)   //了解strstr函数功能！
				{
					char usefullBuffer[2]; 
					switch(i)    //这里的i的case只用选自己需要的数据编号来获取数据
					{
						case 1:memcpy(Save_Data.UTCTime, subString, subStringNext - subString);break;	//获取UTC时间
						case 2:memcpy(usefullBuffer, subString, subStringNext - subString);break;	//判断是否为定位信息
						case 3:memcpy(Save_Data.latitude, subString, subStringNext - subString);break;	//获取纬度信息
						case 4:memcpy(Save_Data.N_S, subString, subStringNext - subString);break;	//获取N/S
						case 5:memcpy(Save_Data.longitude, subString, subStringNext - subString);break;	//获取经度信息
						case 6:memcpy(Save_Data.E_W, subString, subStringNext - subString);break;	//获取E/W

						default:break;
					}

					subString = subStringNext;  //更新subString位置
					Save_Data.isParseData = true;
					if(usefullBuffer[0] == 'A')
                    {
                        Save_Data.isUsefull = true;
                        useful_flag = true;
                    }
						
					else if(usefullBuffer[0] == 'V')
                    {
                        Save_Data.isUsefull = false;
                        useful_flag = false;
                    }
						

				}
				else
				{
					errorLog(2);	//解析错误
				}
			}
            
           

		}
	}
    
    if (Save_Data1.isGetData1)
	{
		Save_Data1.isGetData1 = false;
        printf(Save_Data1.GPS_Buffer1);
		
		for (i = 0 ; i <= 7 ; i++)
		{
			if (i == 0)
			{
				if ((subString1 = strstr(Save_Data1.GPS_Buffer1, ",")) == NULL)
					errorLog(1);	//解析错误
			}
			else
			{
				subString1++;
				if ((subStringNext1 = strstr(subString1, ",")) != NULL)
				{
					switch(i)
					{
						case 1:memcpy(Save_Data1.Newsnum, subString1, subStringNext1 - subString1);break;	
						case 2:memcpy(Save_Data1.Number, subString1, subStringNext1 - subString1);break;	
						case 3:
                            if(Save_Data1.Number[0]=='1')   //字符形式！！！不是数字！！！
                            {
                                if(subStringNext1!=subString1)
                                memcpy(Save_Data1.Weixinnum, subString1, subStringNext1 - subString1);
                                else Save_Data1.Weixinnum[0]='0', Save_Data1.Weixinnum[1]='0';
                            }
                               
                            break;
						case 7:
                            if(Save_Data1.Number[0]=='1')   //判断是否为第一条GPGSV类报文
                            {
                                if(subStringNext1!=subString1)
                                memcpy(Save_Data1.Xinhao, subString1, subStringNext1 - subString1);
                                else Save_Data1.Xinhao[0]='0', Save_Data1.Xinhao[1]='0';
                            }
                                
                            break;

						default:break;
					}

					subString1 = subStringNext1;
//					if(Save_Data1.Number[0]=='1')
                        Save_Data1.isParseData1 = true;
//                    else Save_Data1.isParseData1 = false;
//                    if(strcmp(Save_Data1.Newsnum,Save_Data1.Number)==0)
//                    {
//                        flag_save1 = 1;
//                    }
				}
				else
				{
					errorLog(2);	//解析错误
				}
			}
            
           

		}
	}
}
/*分条解析报文数据*/
void printGpsBuffer()
{
    float latitude_dot,longitude_dot;  //用于经纬度的秒单位运算
	if (Save_Data.isParseData)
	{
        u8 m,y;
		Save_Data.isParseData = false;
        Save_Data.UTCTime[1]=Save_Data.UTCTime[1]-'0'+8;    //算上时差变为北京时间
        if(Save_Data.UTCTime[1]>=10)
        {
            Save_Data.UTCTime[1]=Save_Data.UTCTime[1]%10+'0';
            Save_Data.UTCTime[0]++;
        }
        else Save_Data.UTCTime[1]+='0'; //转为字符形式！！
        
        if(Save_Data.UTCTime[0]=='2' && Save_Data.UTCTime[1]>='4')
        {
            Save_Data.UTCTime[0] = '0';
            Save_Data.UTCTime[1] -= 4;
        }
        
        for(y=m=0;y<=7;y++)
        {
            if(y==2 || y==5)
                Time[y] = ':';
            else Time[y] = Save_Data.UTCTime[m++];
        }
                
        Time[y] = '\0';
        
		if(Save_Data.isUsefull) //用来判断经纬度数据是否有效的一个变量，不用的话下面else里的printf可删
		{
            u8 j;
            u16 k=1;
            Save_Data.isUsefull = false;
            latitude_dot = longitude_dot = 0;
            
            for(j=8,k=1;j>=5;j--)
            {
                latitude_dot+=(Save_Data.latitude[j]-'0')*k;
                k*=10;
            }
            
            latitude_dot /= 10000;
            latitude_dot += (Save_Data.latitude[2]-'0')*10 + (Save_Data.latitude[3]-'0');
            latitude_dot = latitude_dot/60*10000;
            Latitude[0] = Save_Data.latitude[0];
            Latitude[1] = Save_Data.latitude[1];
            Latitude[2] = '.';
            for(j=3,k=1000;j<=6;j++)
            {
                Latitude[j] = (int)latitude_dot/k%10 + '0';
                k/=10;
            }
            Latitude[7] = '\0';
            printf(Latitude);
            printf("\r\n");
            
            for(j=9,k=1;j>=6;j--)
            {
                longitude_dot+=(Save_Data.longitude[j]-'0')*k;
                k*=10;
            }
            
            longitude_dot /= 10000;
            longitude_dot += (Save_Data.longitude[3]-'0')*10 + (Save_Data.longitude[4]-'0');
            longitude_dot = longitude_dot/60*10000;
            Longitude[0] = Save_Data.longitude[0];
            Longitude[1] = Save_Data.longitude[1];
            Longitude[2] = Save_Data.longitude[2];
            Longitude[3] = '.';
            for(j=4,k=1000;j<=7;j++)
            {
                Longitude[j] = (int)longitude_dot/k%10 + '0';
                k/=10;
            }
            Longitude[8] = '\0';
            printf(Longitude);
            printf("\r\n");

		}
		else
		{
			printf("GPS DATA is not usefull!\r\n"); //用来判断数据是否有效的
		}
		
	}
    
    if(Save_Data1.isParseData1)
    {
        Save_Data1.isParseData1 = false;
        strcpy(Weixingnum,Save_Data1.Weixinnum);
        strcpy(Xinhao,Save_Data1.Xinhao);
        printf(Weixingnum);
        printf("\r\n");
        printf(Xinhao);
        printf("\r\n");
    }
}
