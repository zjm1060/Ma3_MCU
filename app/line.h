/*
 * line.h
 *
 *  Created on: 2019Äê4ÔÂ16ÈÕ
 *      Author: zjm09
 */

#ifndef LINE_H_
#define LINE_H_

#include <stdint.h>

typedef struct{
	union{
		struct{
			uint32_t bits:24;
			uint32_t delay:8;
		};
		uint32_t byte;
	};
	uint8_t bitmaps[3200];	// 25600 bits
}line_t;

void line_init(void *ptr,void (*lock)(),void (*unlock)());
line_t *getLine(void);
void freeLine(void *l);

#endif /* LINE_H_ */
