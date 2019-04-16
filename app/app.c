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
int xmodemReceive(unsigned char *dest, int destsz,uint32_t *recvLen);

static SemaphoreHandle_t xSendStart;
static SemaphoreHandle_t xSendStop;
bmp_ctx ctx;
uint8_t *line = 0x30000000;
static uint32_t recvLen;

void appSend(void *args);

osThreadDef(sendTask, appSend, osPriorityNormal, 0, 512);

void startRun(void)
{
	xSemaphoreGive(xSendStart);
}

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
//	  spi_send(line,32*8);
	  HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,1);
	  recvLen = 0;
	  xmodemReceive((char *)0x24000000,(512*1024),&recvLen);
	  osDelay(100);
	  HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,0);
	  xSemaphoreTake(xSendStop,portMAX_DELAY);
	  while(0);
  }
  /* USER CODE END StartDefaultTask */
}

void appSend(void *args)
{
	while(1){
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
			getBmpColor(&ctx,line,i,1);
			spi_send(line,ctx.width);
			osDelay(10);
		}

		xSemaphoreGive(xSendStop);
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

