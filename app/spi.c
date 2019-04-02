/*
 * spi.c
 *
 *  Created on: 2019��4��2��
 *      Author: zjm09
 */

#include "spi.h"

void spi_send(uint8_t *dp,int len)
{
	len >>= 3;
	HAL_SPI_Transmit_DMA(&hspi1,dp,len);
}


