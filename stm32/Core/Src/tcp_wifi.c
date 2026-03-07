#include "tcp_wifi.h"

//AT指令的发送
//参数1: AT命令    参数2: 超时检测 
uint32_t wifi_config(char *cmd,uint16_t time)
{
	  //将AT命令发送到串口5
     u5_printf("%s\r\n",cmd);

	while(time--)
	{
	   if(strstr((char *)RecvBuf,"OK")!= NULL)
	   {
	       break;
	   }
	   HAL_Delay(50);
     }
	 memset(RecvBuf,0,sizeof(RecvBuf));
	 
	 //如果检测到OK, 则返回0 表示成功
	 if(time > 0)
	 {
	    return 0;
	 }
	 else  //未检测到OK, 返回-1, 表示失败
	 {
	    return -1;
	 }	 
}

//wifi连接
uint32_t wifi_connect(char * ssid,char * password)
{
     u5_printf("AT+CWJAP=\"%s\",\"%s\"\r\n",ssid,password);
	 uint16_t time = 300;
	 uint16_t i = 0;
	 while(time--)
	 {
	     if(strstr((char *)RecvBuf,"OK") != NULL)
		 {
		    break;
		 }
		 u1_printf("wifi connect.......%d\n",i++);
		 HAL_Delay(100);
	 }
	  memset(RecvBuf,0,sizeof(RecvBuf));
	  //如果检测到OK, 则返回0 表示成功
	 if(time > 0)
	 {
	    return 0;
	 }
	 else  //未检测到OK, 返回-1, 表示失败
	 {
	    return -1;
	 }	
}

//连接TCP服务器
uint32_t wifi_connecTCP(char *IP,int Port)
{
     u5_printf("AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",IP,Port);
	  uint16_t time = 70;
	  uint16_t i = 0;
	 while(time--)
	 {
		 if(strstr((char *)RecvBuf,"OK") != NULL)
		 {
		     break;
		 }
		 u1_printf("TCP connect.......%d\n",i++);
	     HAL_Delay(100);
	 }
	 	  //如果检测到OK, 则返回0 表示成功
	 if(time > 0)
	 {
	    return 0;
	 }
	 else  //未检测到OK, 返回-1, 表示失败
	 {
	    return -1;
	 }
}

uint32_t TCP_send(char *data)
{
   u5_printf("%s",data);
	return 0;
}