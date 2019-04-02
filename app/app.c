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

void spi_send(uint8_t *dp,int len);
int xmodemReceive(unsigned char *dest, int destsz);

static SemaphoreHandle_t xSendStart;
static SemaphoreHandle_t xSendStop;
bmp_ctx ctx;
uint8_t line[1024];

void appSend(void *args);

osThreadDef(sendTask, appSend, osPriorityNormal, 0, 512);

void StartDefaultTask(void const * argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  HAL_PWREx_EnableUSBVoltageDetector();

  xSendStart = xSemaphoreCreateBinary();
  xSendStop = xSemaphoreCreateBinary();

  osThreadCreate(osThread(sendTask), NULL);

  for(;;)
  {
//	  uart_send("start ....!!!\n\r");
	  xmodemReceive((char *)0x24000000,(512*1024));
	  int offset = getBmpHeader((void *)0x24000000, &ctx);
	  if(offset){
		  offset = getBmpInfo((void *)(0x24000000+offset), &ctx);
		  if(offset == 40){
			  xSemaphoreGive(xSendStart);
		  }
	  }
	  osDelay(100);
	  xSemaphoreTake(xSendStop,portMAX_DELAY);
  }
  /* USER CODE END StartDefaultTask */
}

void appSend(void *args)
{
	while(1){
		xSemaphoreTake(xSendStart,portMAX_DELAY);
		while(1){
			for (int i = 0; i < ctx.height; ++i) {
				  getBmpColor(&ctx,line,i,1);
				  spi_send(line,ctx.width);
				  osDelay(20);
			}
			xSemaphoreGive(xSendStop);
		}
	}
}
