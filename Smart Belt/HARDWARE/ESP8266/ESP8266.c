#include "stm32f10x.h"
#include "ESP8266.h"
#include "OLED.h"
#include "delay.h"
#include "usart.h"
extern char Receive_Buff[256];
extern uint8_t countA;
uint16_t i;

char AT_TEST[]="AT";
char AT_StaMode[]="AT+CWMODE=1";
char AT_MQTTCONNECT[]="AT+MQTTCONN=0,\"121.36.247.156\",1883,0";

FlagStatus ESP8266_Test(void)
{
	Serial_SendString(AT_TEST,SET);     //由串口向WIFI发送AT指令
	delay_ms(10);
	if(Receive_Buff[countA-4]=='O'&&Receive_Buff[countA-3]=='K')    //WIFI响应会向串口发送相应字符串，因为每次发完之后都有串口接收中断复制到Receive_Buff数组，同时导致countA++，所以"OK"所在位置需要特殊形式判断
	{
		return SET;
	}else
	{
		return RESET;
	}
}
FlagStatus ESP8266_STAMODE(void)
{
	Serial_SendString(AT_StaMode,SET);
	delay_ms(10);
	if(Receive_Buff[countA-4]=='O'&&Receive_Buff[countA-3]=='K')
	{
		return SET;
	}else
	{
		return RESET;
	}
}
FlagStatus ESP8266_MQTTCONNECT(void)
{
	Serial_SendString(AT_MQTTCONNECT,SET);
	delay_s(5);
	if(Receive_Buff[countA-4]=='O'&&Receive_Buff[countA-3]=='K')
	{
		return SET;
	}else
	{
		return RESET;
	}
}
void ESP8266_Setup(void)
{
	uart_init(115200);
	delay_s(2);
	if(ESP8266_Test())OLED_ShowString(1, 1, "TestOK",16),OLED_Refresh_Gram();
	delay_s(1);
	if(ESP8266_STAMODE())OLED_ShowString(1, 16, "MODESETOK",16),OLED_Refresh_Gram();
	ESP8266_WIFICONNECT(jibai,jibaiiii);
	delay_s(2);
	ESP8266_MQTTConfig(ID2,147258,147258);
	delay_s(2);
	if(ESP8266_MQTTCONNECT())OLED_ShowString(1, 32, "MQTTCONNECT",16),OLED_Refresh_Gram();
	Serial_SendString("AT+MQTTPUB=0,\"ESP8266\",\"{\\\"longitude\\\":\\\"120\\\"\\,\\\"latitude\\\":\\\"30\\\"\\,\\\"fall\\\":0}\",0,0",SET);
	delay_s(2);
	Serial_SendString("AT+MQTTSUB=0,\"MQTT\",0",SET);
	delay_s(1);
	for(i=0;i<256;i++)Receive_Buff[i]=0;
	OLED_Clear();
}
void ESP8266_Sendcondition(char *longitude,char *latitude,int fall)
{
	Serial_SendMessage("AT+MQTTPUB=0,\"ESP8266\",\"{\\\"longitude\\\":\\\"%s\\\"\\,\\\"latitude\\\":\\\"%s\\\"\\,\\\"fall\\\":%d}\",0,0",longitude,latitude,fall);
}

void ESP8266_recall(char *number)
{
	Serial_SendMessage("AT+MQTTPUB=0,\"RECALL\",\"{\\\"number\\\":\\\"%s\\\"}\",0,0",number);
}
