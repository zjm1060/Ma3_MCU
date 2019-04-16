/*
 * app.c
 *
 *  Created on: 2019Äê4ÔÂ2ÈÕ
 *      Author: zjm09
 */
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app/bmp/bmp.h"
#include "line.h"

#define FRAME_HEADER	(0x68)
#define FRAME_CMD_DATA	(0x01)
#define FRAME_CMD_INFO	(0x02)

#define FRAME_RESULT_ACK	(0x00)
#define FRAME_RESULT_NAK	(0xFF)
#define FRAME_RESULT_ERR	(0xEE)

typedef struct{
	uint8_t h;
	uint8_t ack;
//	line_t data;
}frame_t;

void spi_send(uint8_t *dp,int len);
int xmodemReceive(unsigned char *dest, int destsz,uint32_t *recvLen);

//static SemaphoreHandle_t xSendStart;
//static SemaphoreHandle_t xSendStop;
static QueueHandle_t xSendQueue;
bmp_ctx ctx;
//uint8_t *line = 0x30000000;
static uint32_t recvLen;
static frame_t result;

void appSend(void *args);

osThreadDef(sendTask, appSend, osPriorityNormal, 0, 128);

//void startRun(void)
//{
//	xSemaphoreGive(xSendStart);
//}

void recvFrame(void)
{
	uint8_t c;

	while(1){
		HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,0);
		getbytes(&c,1);
		HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,1);

		if(c == FRAME_HEADER){
			getbytes(&c,1);
			switch(c){
				case FRAME_CMD_DATA:{
					line_t *line = getLine();
					if(line){
						getbytes(&line->bits,sizeof(line->bits));
						if((line->bits>>3) > 1500){
							result.h = FRAME_HEADER;
							result.ack = FRAME_RESULT_ERR;
							uart_send_bytes(&result,sizeof(result));
						}

						getbytes(line->bitmaps,(line->bits>>3));

						xQueueSend(xSendQueue,&line,portMAX_DELAY);

						result.h = FRAME_HEADER;
						result.ack = FRAME_RESULT_ACK;
						uart_send_bytes(&result,sizeof(result));
					}else{
						while (_inbyte((100>>1)) >= 0);
						result.h = FRAME_HEADER;
						result.ack = FRAME_RESULT_NAK;
						uart_send_bytes(&result,sizeof(result));
					}
				}break;
				case FRAME_CMD_INFO:{
					uint8_t waiting = uxQueueMessagesWaiting(xSendQueue);
					result.h = FRAME_HEADER;
					result.ack = waiting;
					uart_send_bytes(&result,sizeof(result));
				}break;
			}
		}
	}
}

void StartDefaultTask(void const * argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  HAL_PWREx_EnableUSBVoltageDetector();

//  xSendStart = xSemaphoreCreateBinary();
//  xSendStop = xSemaphoreCreateBinary();

  xSendQueue = xQueueCreate(128,sizeof(void *));

  line_init((void *)0x38000000);

  osThreadCreate(osThread(sendTask), NULL);

  for(;;)
  {
//	  spi_send(line,32*8);
//	  HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,1);
//	  recvLen = 0;
//	  xmodemReceive((char *)0x24000000,(512*1024),&recvLen);
//	  osDelay(100);

	  recvFrame();

//	  HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,0);
//	  xSemaphoreTake(xSendStop,portMAX_DELAY);
//	  while(0);
  }
  /* USER CODE END StartDefaultTask */
}

void appSend(void *args)
{
#if 0
	while(0){
		xSemaphoreTake(xSendStart,portMAX_DELAY);

		while(recvLen < 54)
			osDelay(10);

		memset(&ctx,0,sizeof(ctx));
		int offset = getBmpHeader((void *)0x24000000, &ctx);
		if(offset){
			offset = getBmpInfo((void *)(0x24000000+offset), &ctx);
			if(offset != 40){
				continue;
			}
		}

		while(recvLen < 54+(ctx.width>>3))
			osDelay(10);

		for (int i = 0; i < ctx.height; ++i) {
//			getBmpColor(&ctx,line,i,1);
//			spi_send(line,ctx.width);
			osDelay(10);
		}

		xSemaphoreGive(xSendStop);
		while(0);
	}
#endif

	while(1){
		line_t *line;

		xQueueReceive(xSendQueue, &line, portMAX_DELAY) ;

		spi_send(line->bitmaps,line->bits);

		freeLine(line);

		osDelay(10);

		while(0);
	}
}

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disables the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU attributes as WT for NOR */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x30000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

