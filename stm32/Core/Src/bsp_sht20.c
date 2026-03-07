/**
  ******************************************************************************
  * @file   bsp_sht20.c
  * @brief  温湿度传感器SHT20驱动
  *   
  ******************************************************************************
  */
#include "bsp_sht20.h"
#include "i2c.h"
#include "stdio.h"
//
volatile SHT20_TemRH_Val gTemRH_Val;	//存储温湿度值
//
/*
**********************************************************************
* @fun     :BSP_SHT20_Read 
* @brief   :根据指令读取sht20内部的寄存器
* @param   :
* @return  :sht20_reg_val 
**********************************************************************
*/
uint16_t BSP_SHT20_Read(uint8_t sht20_cmd)
{
		uint16_t sht20_reg_val = 0;
		uint8_t sht20_reg_buff[2]={0x00,0x00};
		HAL_I2C_Master_Transmit(&hi2c1,SHT20_ADDR_WRITE,&sht20_cmd,1,100);    //发送控制指令
		HAL_Delay(10);
		HAL_I2C_Master_Receive(&hi2c1,SHT20_ADDR_READ,sht20_reg_buff,2,100);      //读取数据，两个字节
		HAL_Delay(10);
		sht20_reg_val=(sht20_reg_buff[0]<<8)|sht20_reg_buff[1];										//合并数据
	
		return (sht20_reg_val);
}
/*
**********************************************************************
* @fun     :BSP_SHT20_GetData 
* @brief   :获取sht20的温湿度数据
* @param   :
* @return  :none
**********************************************************************
*/
void BSP_SHT20_GetData(void)
{
	uint16_t  pTem=0,pHum=0;
	//读取数据
	pTem=BSP_SHT20_Read(SHT20_HOLD_M_READ_T);
	pHum=BSP_SHT20_Read(SHT20_HOLD_M_READ_RH);
	//数据转换
	gTemRH_Val.Tem	= -46.85f+175.72f*((float)pTem/65536);
	gTemRH_Val.Hum  = -6+125*((float)pHum/65536);
	//printf("temperature:%.2f, humidity:%.2f\n",gTemRH_Val.Tem,gTemRH_Val.Hum);
}

