/*
 * uart.c
 *
 *  Created on: 2019Äê4ÔÂ2ÈÕ
 *      Author: zjm09
 */
#include "cmsis_os.h"
#include <stdint.h>

extern osMessageQId q_UartRecvHandle;

void cdc_recv(uint8_t *data,int len)
{
	portBASE_TYPE taskWoken = pdFALSE;

	for (int i = 0; i < len; ++i) {

	    if (xQueueSendFromISR(q_UartRecvHandle, &data[i], &taskWoken) != pdTRUE) {
	      return ;
	    }

//		osMessagePut(q_UartRecvHandle,data[i],0);
	}

	portEND_SWITCHING_ISR(taskWoken);
}

int _inbyte(unsigned short timeout)
{
//	osEvent e = osMessageGet(q_UartRecvHandle,timeout);
//
//	if(e.status == osEventMessage){
//		return e.value.v;
//	}

	TickType_t ticks = timeout / portTICK_PERIOD_MS;
	uint8_t ch;

    if (xQueueReceive(q_UartRecvHandle, &ch, ticks) == pdTRUE) {
      /* We have mail */
      return (int)ch;
    }

	return -1;
}

uint8_t _getbyte(void)
{
	uint8_t ch = -1;

	xQueueReceive(q_UartRecvHandle, &ch, portMAX_DELAY) ;

	return ch;

}

int getbytes(uint8_t *data,int len)
{
	for (int i = 0; i < len; ++i) {
		data[i] = _getbyte();
	}
}

int _isempty(void)
{
	uint8_t ch = -1;

	if(xQueuePeek(q_UartRecvHandle,&ch,0) == pdTRUE){
		return 1;
	}

	return 0;
}

void uart_send(const char *str)
{
	CDC_Transmit_FS(str,strlen(str));
}

void uart_send_bytes(uint8_t *bytes,int len)
{
	CDC_Transmit_FS(bytes,len);
}
