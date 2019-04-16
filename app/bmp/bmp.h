/*
 * bmp.h
 *
 *  Created on: 2019Äê4ÔÂ2ÈÕ
 *      Author: zjm09
 */

#ifndef BMP_H_
#define BMP_H_

#include <stdint.h>

typedef struct{
	void *bmp_data;
	void *color_tab;
	uint32_t file_size;
	uint32_t data_offset;
	uint32_t width;
	uint32_t height;
	uint32_t bits;
	uint32_t compression;
}bmp_ctx;

//typedef union{
//	struct{
//		uint8_t b,g,r,a;
//	}c;
//	uint32_t Monochrome;
//}tColor;

int getBmpHeader(void *h, bmp_ctx *ctx);
int getBmpInfo(void *inf, bmp_ctx *ctx);
void getBmpColor(bmp_ctx *ctx, uint8_t *dp, int start_line,int lines);

#endif /* BMP_H_ */
