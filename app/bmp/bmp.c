/*
 * bmp.c
 *
 *  Created on: 2019Äê4ÔÂ2ÈÕ
 *      Author: zjm09
 */
#include <string.h>
#include "bmp.h"

#define GrayLimte	(200)

#define __PACKED __attribute__((__packed__))

typedef struct
{
    uint8_t  magic[2];    /* Magic bytes 'B' and 'M'. */
    uint32_t file_size;   /* Size of whole file. */
    uint32_t unused;      /* Should be 0. */
    uint32_t data_offset; /* Offset from beginning of file to bitmap data. */
}__PACKED bmp_header;

typedef struct{
	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t q;
}__PACKED color_tab;

typedef union{
	color_tab c;
	uint32_t color;
}tColor;

typedef struct bmp_info
{
    uint32_t info_size;   /* Size of info struct (> sizeof(bmp_info)). */
    int32_t  width;       /* Width of image. */
    int32_t  height;      /* Height (< 0 means right-side up). */
    uint16_t planes;      /* Planes (should be 1). */
    uint16_t bits;        /* Number of bits (1, 4, 8, 16, 24, or 32). */
    uint32_t compression; /* See COMPRESSION_* values below. */
    uint32_t unused0[3];  /* We don't care about these fields. */
    uint32_t colors;      /* How many colors in the palette, 0 = 1<<bits. */
    uint32_t unused1;     /* Another field we don't care about. */
//    uint32_t masks[4];    /* Bitmasks for 16- and 32-bit images. */
    color_tab tab[0];
    /* There can be additional later fields in the actual file info, but we
     * don't need them here.
     */

}__PACKED bmp_info;

int getBmpHeader(void *h, bmp_ctx *ctx)
{
	bmp_header *header = h;
	if(header->magic[0] != 0x42 || header->magic[1] != 0x4d)
		return -1;

//	ctx->bmp_data = h;
	ctx->file_size = header->file_size;
	ctx->data_offset = header->data_offset;

	ctx->bmp_data = h + ctx->data_offset;

	return sizeof(bmp_header);
}

#define COMPRESSION_NONE      0
#define COMPRESSION_RLE8      1
#define COMPRESSION_RLE4      2
int getBmpInfo(void *inf, bmp_ctx *ctx)
{
	bmp_info *info = inf;
	if(info->info_size < 40) return -1;
	if(info->planes != 1)return -1;
	if(info->bits != 1 &&
	   info->bits != 4 &&
	   info->bits != 8 &&
	   info->bits != 16 &&
	   info->bits != 24 &&
	   info->bits != 32)return -1;
	if(info->compression != COMPRESSION_NONE &&
	   info->compression != COMPRESSION_RLE8 &&
	   info->compression != COMPRESSION_RLE4)return -1;

	ctx->width = info->width;
	ctx->height = info->height;

	ctx->bits = info->bits;
	ctx->compression = info->compression;
	ctx->color_tab = NULL;

	if(ctx->bits < 16){
		ctx->color_tab = info->tab;
//		ctx->bmp_data += (ctx->bits*2)*4;
	}

	return sizeof(bmp_info);
}

int getBmpMonochrome(bmp_ctx *ctx, int x, int y)
{

}

#define toGray(c)	(((c)->r*76 + (c)->g*150 + (c)->b*30) >> 8)

void getBmpColor(bmp_ctx *ctx, uint8_t *dp, int start_line, int lines)
{
	int byte_in_line = 0;
	int offset_in_line = 0;
	uint8_t *line = NULL;
	switch(ctx->bits){
		case 1:{
			byte_in_line = ctx->width>>3;
			for (int i = 0; i < lines; ++i) {
				line = (uint8_t *)(((uint8_t *)ctx->bmp_data)+((start_line+i)*byte_in_line));
				memcpy(dp,line,byte_in_line);
			}
		}break;
		case 4:{
			color_tab *tab = ctx->color_tab;
			tColor c;
			int m;
			uint8_t byte = 0;
			int bitCount = 7;

			byte_in_line = ctx->width>>1;

			for (int i = 0; i < lines; ++i) {
				line = (uint8_t *)(((uint8_t *)ctx->bmp_data)+((start_line+i)*byte_in_line));
				for (int x = 0; x < byte_in_line; ++x) {
					uint8_t index = line[x]>>4;
					c.c = tab[index];
					m = toGray(&c.c);
					if(m > GrayLimte)byte |= (1<<bitCount);
					bitCount --;
					index = line[x]&0xf;
					c.c = tab[index];
					m = toGray(&c.c);
					if(m > GrayLimte)byte |= (1<<bitCount);
					bitCount --;

					if(bitCount <= 0){
						*dp = byte;
						dp ++;
						byte = 0;
						bitCount = 7;
					}
				}
			}

		}break;
		case 8:{
			byte_in_line = ctx->width;
			color_tab *tab = ctx->color_tab;
			int m;
			tColor c;
			uint8_t byte = 0;
			int bitCount = 7;

			for (int i = 0; i < lines; ++i) {
				line = (uint8_t *)(((uint8_t *)ctx->bmp_data)+((start_line+i)*byte_in_line));
				for (int x = 0; x < byte_in_line; ++x) {
					uint8_t index = line[x];
					c.c = tab[index];
					m = toGray(&c.c);
					if(m > GrayLimte)byte |= (1<<bitCount);
					bitCount --;

					if(bitCount < 0){
						*dp = byte;
						dp ++;
						byte = 0;
						bitCount = 7;
					}
				}
			}
		}break;
		case 16:{
			byte_in_line = ctx->width>>1;

		}break;
		case 24:{
			byte_in_line = ctx->width*3;
			int m;
			tColor c;
			uint8_t byte = 0;
			int bitCount = 7;

			for (int i = 0; i < lines; ++i) {
				line = (uint8_t *)(((uint8_t *)ctx->bmp_data)+((start_line+i)*byte_in_line));
				for (int x = 0; x < byte_in_line; x+=3) {
					c.c.b = line[x+0];
					c.c.g = line[x+1];
					c.c.r = line[x+2];
					m = toGray(&c.c);
					if(m > GrayLimte)byte |= (1<<bitCount);
					bitCount --;

					if(bitCount < 0){
						*dp = byte;
						dp ++;
						byte = 0;
						bitCount = 7;
					}
				}
			}
		}
		default:
			break;
	}

}



