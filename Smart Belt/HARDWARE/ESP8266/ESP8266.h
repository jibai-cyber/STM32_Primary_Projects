#ifndef _ESP8266_H
#define _ESP8266_H

#include "usart.h"
/*WIFI连接*/
#define ESP8266_WIFICONNECT(WIFINAME,PASSWORD) Serial_SendString("AT+CWJAP=\""#WIFINAME"\",\""#PASSWORD"\"",SET)
/*用户配置，client_id随意，账户密码可以使用147258*/
#define ESP8266_MQTTConfig(client_id,Account,password) Serial_SendString("AT+MQTTUSERCFG=0,1,\""#client_id"\",\""#Account"\",\""#password"\",0,0,\"\"",SET)
/*测试AT模块是否正常*/
FlagStatus ESP8266_Test(void);
/*设置为station模式*/
FlagStatus ESP8266_STAMODE(void);
/*连接MQTT服务器*/
FlagStatus ESP8266_MQTTCONNECT(void);

/*第一次开机初始化*/
void ESP8266_Setup(void);
/*初始化后的数据发送*/
void ESP8266_Sendcondition(char *longitude,char *latitude,int fall);

void ESP8266_recall(char *number);
#endif
