/**
  ******************************************************************************
  * @file   bsp_sht20.h
  * @brief  温湿度传感器SHT20驱动
  *          
  ******************************************************************************
  */
#ifndef __BSP_SHT20_H__
#define __BSP_SHT20_H__
//
#include "main.h"

#define SHT20_ADDR 0x40

#define SHT20_ADDR_READ  0x81
#define SHT20_ADDR_WRITE 0x80


#define SHT20_HOLD_M_READ_T 		0xE3    //1110’0011 
#define SHT20_HOLD_M_READ_RH 		0xE5    //1110’0101
#define SHT20_NOHOLD_M_READ_T 	0xF3  	//1111’0011
#define SHT20_NOHOLD_M_READ_RH 	0xF5  	//1111’0101
//温湿度传感器数据结构
typedef struct
{
	float	Tem;
  float Hum;
}SHT20_TemRH_Val; 
//
uint16_t BSP_SHT20_Read(uint8_t sht20_cmd);
void BSP_SHT20_GetData(void);
//
#endif /* __BSP_SHT20_H__ */

