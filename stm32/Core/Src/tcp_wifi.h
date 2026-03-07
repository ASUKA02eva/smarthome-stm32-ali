#ifndef _WIFI_H_
#define _WIFI_H_

#include "stdio.h"
#include "usart.h"
#include "string.h"

extern uint8_t RecvBuf[2048];


uint32_t wifi_config(char *cmd,uint16_t time);
uint32_t wifi_connect(char * ssid,char * password);
uint32_t wifi_connecTCP(char *IP,int Port);
uint32_t TCP_send(char *data);
#endif