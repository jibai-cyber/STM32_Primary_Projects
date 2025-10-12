#include "stm32f10x.h" 
#include "SYN6288.h"
#include <string.h>
#include <stdlib.h>
#include "usart.h"
char broadcast[128];
void TTSPlay(uint8_t sound,char *Text,u8 volume)
{
  uint8_t i=0,xorcrc=0,uLen;
  uint8_t SoundBuf[256];						//操作语音提示缓冲 
	char *p = NULL;
	p = UTFchange(Text,&uLen);
  for(i=0;i<100;i++)SoundBuf[i]=0x00;
  SoundBuf[0]=0xFD;
  SoundBuf[1]=0x00;
  SoundBuf[2]=uLen+25;          
  SoundBuf[3]=0x01;
	SoundBuf[4]=0x03;
  SoundBuf[5]=0x00;
	SoundBuf[6]=0x5B;
  SoundBuf[7]=0x00;
  SoundBuf[8]=0x76;          
  SoundBuf[9]=0x00;
  SoundBuf[10]=volume/10+'0';
	SoundBuf[11]=0x00;
  SoundBuf[12]=volume%10+'0';
  SoundBuf[13]=0x00;          
  SoundBuf[14]=0x5D;
  SoundBuf[15]=0x00;
	SoundBuf[16]=0x73;
	SoundBuf[17]=0x00;
	SoundBuf[18]=0x6F;
	SoundBuf[19]=0x00;
	SoundBuf[20]=0x75;
	SoundBuf[21]=0x00;
	SoundBuf[22]=0x6E;
	SoundBuf[23]=0x00;
	SoundBuf[24]=0x64;
	SoundBuf[25]=0x00;
	SoundBuf[26]=sound;
  for (i=0;i<uLen;i++)
  {
    SoundBuf[27+i]=p[i];
  }
	free(p);
  for (i=0;i<uLen+27;i++)
  {
		xorcrc=xorcrc ^ SoundBuf[i];
		UART2_SendByte(SoundBuf[i]);
  }
  UART2_SendByte(xorcrc);
}
/***************************************************************************
UFT-8格式转换Unicode格式函数
***************************************************************************/
char *UTFchange(char* Data,uint8_t *size)
{
  uint8_t Out_put[256];    					//暂定最多可改64位的字符串
  int i;
  int len_Str = strlen(Data);    		//得到UTF-8字符串长度
  int Label_Record = 0;
  char *Out_put_Fin;
  memset(Out_put,0,sizeof(Out_put));

  for(i = 0 ; i < len_Str ; i++)
  {
    if(Data[i]/128 == 0)  					//如果除以127>0代表首位为1
    {
      unsigned char unicode_character[2];
      unicode_character[0] = 0x00;       					//高字节填0x00
      unicode_character[1] = Data[i];    					//低字节不变
      Out_put[Label_Record] = unicode_character[0];  //
      Label_Record++;
      Out_put[Label_Record] = unicode_character[1];
      Label_Record++;
    }
    //首位不为1的情况，也就是说遇到了中文字符
    //此项目为喇叭字符转义驱动，中文字符首位在1110 0100到1110 1001 之间（其实已经囊括绝大多数汉字），所以不在这个区间的直接报错
    else
    {
      //if(Data[i]/16 == 14 && Data[i]%16 > 4 && Data[i]%16 < 9 )    //这里就代表这个字节的高四位是1110,并且低位在一定的范围内
      if(Data[i]/16 == 14)
      {
        unsigned char  High_byte = Data[i+0];    //代表中文UTF-8高位
        unsigned char  Middle_byte = Data[i+1];  //代表中文UTF-8中位
        unsigned char  Low_byte = Data[i+2];     //代表中文UTF-8低位
        unsigned short temp,Unicode_Chinese_High_Byte,Unicode_Chinese_Low_Byte;
          
        High_byte = High_byte % 16;    //高位取余16，就是抹掉高四位的1110
        Middle_byte = Middle_byte % 64;  //中位取余64，就是抹掉高两位的10
        Low_byte = Low_byte % 64;  //低位取余64，就是抹掉高两位的10

        temp = 0;
        temp = High_byte*4096 + Middle_byte*64 + Low_byte;

        Unicode_Chinese_High_Byte = temp/256;
        Unicode_Chinese_Low_Byte = temp%256;
        Out_put[Label_Record] = Unicode_Chinese_High_Byte;
        Label_Record++;
        Out_put[Label_Record] = Unicode_Chinese_Low_Byte;
        Label_Record++;
      }
      i = i + 2;
    }
  }
	*size = Label_Record;
	Out_put_Fin = (char *)malloc(Label_Record);
	//这里建立一个大小刚好与建立好的unicode形式字符串大小的字符串
  for(i = 0 ; i < Label_Record ; i++)
    Out_put_Fin[i] = Out_put[i];
	return Out_put_Fin;
}

