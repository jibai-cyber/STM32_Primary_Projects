#include "oledfont.h"
#include "oled1.h"

u8 OLED_GRAM[128][8]; 
/*���ų�ʼ��*/
void OLED_I2C_Init(void)
{
     GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
 

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;  //����ע�����ſ��ܲ�ͬ
  GPIO_Init(GPIOB, &GPIO_InitStructure);
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
 
 OLED_W_SCL(1);
 OLED_W_SDA(1);
}

/**
  * @brief  I2C��ʼ
  * @param  ��
  * @retval ��
  */
void OLED_I2C_Start(void)
{
 OLED_W_SDA(1);
 OLED_W_SCL(1);
 OLED_W_SDA(0);
 OLED_W_SCL(0);
}

/**
  * @brief  I2Cֹͣ
  * @param  ��
  * @retval ��
  */
void OLED_I2C_Stop(void)
{
 OLED_W_SDA(0);
 OLED_W_SCL(1);
 OLED_W_SDA(1);
}

/**
  * @brief  I2C����һ���ֽ�
  * @param  Byte Ҫ���͵�һ���ֽ�
  * @retval ��
  */
void OLED_I2C_SendByte(uint8_t Byte)
{
 uint8_t i;
 for (i = 0; i < 8; i++)
 {
  OLED_W_SDA(Byte & (0x80 >> i));
  OLED_W_SCL(1);
  OLED_W_SCL(0);
 }
 OLED_W_SCL(1); //�����һ��ʱ�ӣ�������Ӧ���ź�
 OLED_W_SCL(0);
}

/**
  * @brief  OLEDд����
  * @param  Command Ҫд�������
  * @retval ��
  */
void OLED_WriteCommand(uint8_t Command)
{
 OLED_I2C_Start();
 OLED_I2C_SendByte(0x78);  //�ӻ���ַ
 OLED_I2C_SendByte(0x00);  //д����
 OLED_I2C_SendByte(Command); 
 OLED_I2C_Stop();
}

/**
  * @brief  OLEDд����
  * @param  Data Ҫд�������
  * @retval ��
  */
void OLED_WriteData(uint8_t Data)
{
 OLED_I2C_Start();
 OLED_I2C_SendByte(0x78);  //�ӻ���ַ
 OLED_I2C_SendByte(0x40);  //д����
 OLED_I2C_SendByte(Data);
 OLED_I2C_Stop();
}

//�����Դ浽OLED  
void OLED_Refresh_Gram(void)
{
 u8 i,n;      
 for(i=0;i<8;i++)  
 {  
   OLED_WriteCommand(0xB0+i);     //����Yλ��
   OLED_WriteCommand(0x10); //����Xλ�ø�4λ
   OLED_WriteCommand(0x00);   //����Xλ�õ�4λ
  for(n=0;n<128;n++)
  {
   OLED_WriteData(OLED_GRAM[n][i]); 
  }
 }   
}

//���� 
//x:0~127
//y:0~63
//t:1 ��� 0,���       
void OLED_DrawPoint(u8 x,u8 y,u8 t)
{
 u8 pos,bx,temp=0;
 if(x>127||y>63)
 {
  return;//������Χ��.
 }
 pos=7-y/8;
 bx=y%8;
 temp=1<<(7-bx);
 if(t)
 {
  OLED_GRAM[x][pos]|=temp;
 }
 else OLED_GRAM[x][pos]&=~temp;     
}

//��ָ��λ����ʾһ���ַ�,���������ַ�
//x:0~127
//y:0~63
//mode:0,������ʾ;1,������ʾ     
//size:ѡ������ 12/16/24
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size,u8 mode)
{             
 u8 temp,t,t1;
 u8 y0=y;
 u8 csize=(size/8+((size%8)?1:0))*(size/2);  //�õ�����һ���ַ���Ӧ������ռ���ֽ������˺�ǰ��ʾ���ֽڣ��˺ź��ʾ���ֽ� eg. size=16->��Ҫ16*8���ֽ�����ʾ size=12->��Ҫ16*6���ֽ�����ʾ������������ܱ�8����������ȡ������Ϊoled��ʾ��ʽΪ���еģ�һ�ֽ�һ�ֽ���ʾ����������Զȡһ��
 chr=chr-' ';//�õ�ƫ�ƺ��ֵ   
    for(t=0;t<csize;t++)
    {   
  if(size==12)temp=asc2_1206[chr][t];    //����1206����
  else if(size==16)temp=asc2_1608[chr][t]; //����1608����
  else if(size==24)temp=asc2_2412[chr][t]; //����2412����
  else return;        //û�е��ֿ�
        for(t1=0;t1<8;t1++)     //ÿ��ȡtemp�����λ��ӡ��һ��ѭ����ӡ8��bit����һ��byte���൱�ڴ�ӡ���ֿ������е�һ��Ԫ��
  {
   if(temp&0x80)OLED_DrawPoint(x,y,mode);
   else OLED_DrawPoint(x,y,!mode);
   temp<<=1;
   y++;
   if((y-y0)==size) //ע�⿴���������ȡֵΪ12���൱�ڴ�ӡ��12�е�ʱ���ֱ������ѭ����ʣ�µ�16-12=4�в�����ʾ���൱������
   {
    y=y0;
    x++;
    break;
   }
  }    
    }          
}

void OLED_Show_ChineseChar(u8 x,u8 y,u8 num,u8 size,u8 mode)
{             
 u8 temp,t,t1;
 u8 y0=y;
 u8 csize=(size/8+((size%8)?1:0))*size;  //�õ�����һ���ַ���Ӧ������ռ���ֽ���
    for(t=0;t<csize;t++)
    {   
        switch(size)
        {
            case 16: temp=Chinese_16[num][t]; break;
            case 24: temp=Chinese_24[num][t]; break;
            default: return;
        }
        for(t1=0;t1<8;t1++)     //ÿ��ȡtemp�����λ��ӡ��һ��ѭ����ӡ8��bit����һ��byte���൱�ڴ�ӡ���ֿ������е�һ��Ԫ��
  {
   if(temp&0x80)OLED_DrawPoint(x,y,mode);
   else OLED_DrawPoint(x,y,!mode);
   temp<<=1;
   y++;
   if((y-y0)==size)
   {
    y=y0;
    x++;
    break;
   }
  }    
    }          
}
//m^n����
u32 mypow(u8 m,u8 n)
{
 u32 result=1;  
 while(n--)result*=m;    
 return result;
} 

//��ʾ2������
//x,y :�������  
//len :���ֵ�λ��
//size:�����С
//mode:ģʽ 0,���ģʽ;1,����ģʽ
//num:��ֵ(0~4294967295);      
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size,u8 mode)
{          
 u8 t,temp;
 u8 enshow=0;         
 for(t=0;t<len;t++)
 {
  temp=(num/mypow(10,len-t-1))%10;
  if(enshow==0&&t<(len-1))
  {
   if(temp==0)
   {
    OLED_ShowChar(x+(size/2)*t,y,' ',size,mode);
    continue;
   }else enshow=1; 
     
  }
   OLED_ShowChar(x+(size/2)*t,y,temp+'0',size,mode); 
 }
} 
//��ʾ�ַ���
//x,y:�������  
//size:�����С 
//*p:�ַ�����ʼ��ַ 
void OLED_ShowString(u8 x,u8 y,const u8 *p,u8 size)
{ 
    while((*p<='~')&&(*p>=' '))//�ж��ǲ��ǷǷ��ַ�!
    {       
        if(x>(128-(size/2))){x=0;y+=size;}
        if(y>(64-size)){y=x=0;OLED_Clear();}
        OLED_ShowChar(x,y,*p,size,1);  
        x+=size/2;
        p++;
    }  
 
}    

void OLED_Show_ChineseString(u8 x,u8 y,const u8 *p,u8 size) //�����������һλ������0�����򲻺��ж�
{ 
    while(*p!=0)//�ж��ǲ��ǷǷ��ַ�!���0Ҳ�������������β
    {       
        if(x>(128-size)){x=0;y+=size;}
        if(y>(64-size)){y=x=0;OLED_Clear();}
        OLED_Show_ChineseChar(x,y,*p,size,1);  
        x+=size;
        p++;
    }  
 
}    
/*graph_num��ʾ�ڼ���Graph���棬size��oledfont.h�ļ�Graph����*/
void OLED_ShowGraph(u8 x,u8 y,u8 Graph_num,u8 ysize,u16 size)
{
     u16 t;
     u8 y0=y,temp,t1;
     for(t=0;t<size;t++)
    {   
        switch(Graph_num)
        {
            case 1:temp = Graph1[t];break;
            case 2:temp = Graph2[t];break;
            case 3:temp = Graph3[t];break;
            case 4:temp = Graph4[t];break;
            case 5:temp = Graph5[t];break;
            default: break;
        }
           
        
            for(t1=0;t1<8;t1++)     
          {
           if(temp&0x80)OLED_DrawPoint(x,y,1);
           else OLED_DrawPoint(x,y,0);
           temp<<=1;
           y++;
           if((y-y0)==ysize)
           {
            y=y0;
            x++;
            break;
           }
          }    
    }   
}


//��������,������,������Ļ�Ǻ�ɫ��!��û����һ��!!!   
void OLED_Clear(void)  
{  
 u8 i,n;  
 for(i=0;i<8;i++)
 {
  for(n=0;n<128;n++)
  {
   OLED_GRAM[n][i]=0X00;  
  }
 }
 OLED_Refresh_Gram();//������ʾ
}

/*�ֲ���������x0��x1��y0��y1��mode������ɫ*/
void OLED_Clear_part(u8 x0,u8 y0,u8 x1,u8 y1,u8 mode)
{
    u8 x,y;
//    if(x0>=x1) {x0 = x_b,x1 = x_s;}
//    else {x0 = x_s,x1 = x_b;}
//    
//    if(y0>=y1) {y0 = y_b,y1 = y_s;}
//    else {y0 = y_s,y1 = y_b;}
    for(x=x0;x<=x1;x++)
        for(y=y0;y<=y1;y++)
            OLED_DrawPoint(x,y,mode);
//    OLED_Refresh_Gram();
}
/*��ת��x0��x1��y0��y1�������ɫ*/
void OLED_Opposite(u8 x0,u8 y0,u8 x1,u8 y1)
{
    u8 x,y,pos,bx,temp;
    for(x=x0;x<=x1;x++)
        for(y=y0;y<=y1;y++)
        {
           if(x>127||y>63)
          {
           return;//������Χ��.
          }
           pos=7-y/8;
           bx=y%8;
           temp=1<<(7-bx);
            if(OLED_GRAM[x][pos]&temp)
                OLED_DrawPoint(x,y,0);
            else OLED_DrawPoint(x,y,1);
        }
    OLED_Refresh_Gram();
}

u8 OLED_Judge(u8 x,u8 y)    //�жϸ�λ�Ǻڻ��ǰף���Ϊ0����Ϊ1������Ϊ2
{
    u8 pos,bx,temp;
    if(x>127||y>63)
      {
       return 2;//������Χ��.
      }
       pos=7-y/8;
       bx=y%8;
       temp=1<<(7-bx);
        if(OLED_GRAM[x][pos]&temp)
            return 1;
        else return 0;
}

//��ʾ����
//x,y:������� 
//num:�ֿ���λ��
//size:����߶� 
//mode:ģʽ 0,���ģʽ;1,����ģʽ
//void OLED_ShowChinese(u8 x,u8 y,u8 num,u8 size,u8 mode)
//{             
// u8 temp,t,t1;
// u8 y0=y;
// u8 csize=32;  //�õ�����һ���ַ���Ӧ������ռ���ֽ���  
//    for(t=0;t<csize;t++)
//    {   
//  if(size==16)temp=Oled_Chinese[num][t]; //����1608����
//  else return;        //û�е��ֿ�
//        for(t1=0;t1<8;t1++)
//  {
//   if(temp&0x80)OLED_DrawPoint(x,y,mode);
//   else OLED_DrawPoint(x,y,!mode);
//   temp<<=1;
//   y++;
//   if((y-y0)==size)
//   {
//    y=y0;
//    x++;
//    break;
//   }
//  }    
//    }          
//}

/**
  * @brief  OLED��ʼ��
  * @param  ��
  * @retval ��
  */
void OLED_Init(void)
{
 uint32_t i, j;
 
 for (i = 0; i < 1000; i++)   //�ϵ���ʱ
 {
  for (j = 0; j < 1000; j++);
 }
 
 OLED_I2C_Init();   //�˿ڳ�ʼ��
 
 OLED_WriteCommand(0xAE); //�ر���ʾ
 
 OLED_WriteCommand(0xD5); //������ʾʱ�ӷ�Ƶ��/����Ƶ��
 OLED_WriteCommand(0x80);
 
 OLED_WriteCommand(0xA8); //���ö�·������
 OLED_WriteCommand(0x3F);
 
 OLED_WriteCommand(0xD3); //������ʾƫ��
 OLED_WriteCommand(0x00);
 
 OLED_WriteCommand(0x40); //������ʾ��ʼ��
 
 OLED_WriteCommand(0xA1); //�������ҷ���0xA1���� 0xA0���ҷ���
 
 OLED_WriteCommand(0xC0); //�������·���0xC8���� 0xC0���·���

 OLED_WriteCommand(0xDA); //����COM����Ӳ������
 OLED_WriteCommand(0x12);
 
 OLED_WriteCommand(0x81); //���öԱȶȿ���
 OLED_WriteCommand(0xCF);

 OLED_WriteCommand(0xD9); //����Ԥ�������
 OLED_WriteCommand(0xF1);

 OLED_WriteCommand(0xDB); //����VCOMHȡ��ѡ�񼶱�
 OLED_WriteCommand(0x30);

 OLED_WriteCommand(0xA4); //����������ʾ��/�ر�

 OLED_WriteCommand(0xA6); //��������/��ת��ʾ

 OLED_WriteCommand(0x8D); //���ó���
 OLED_WriteCommand(0x14);

 OLED_WriteCommand(0xAF); //������ʾ
  
 OLED_Clear();    //OLED����
}



