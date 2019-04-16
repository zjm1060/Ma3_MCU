/*
 * spi.c
 *
 *  Created on: 2019Äê4ÔÂ2ÈÕ
 *      Author: zjm09
 */

#include "spi.h"

void spi_send(uint8_t *dp,int len)
{
	len >>= 3;
//	HAL_SPI_Transmit_DMA(&hspi2,0x30000000,len);
	HAL_SPI_Transmit(&hspi2,dp,len,100);
}


