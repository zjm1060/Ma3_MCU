/*
 * line.c
 *
 *  Created on: 2019Äê4ÔÂ16ÈÕ
 *      Author: zjm09
 */

#include <string.h>
#include "line.h"

#define LINE_POOL_SIZE (16)		// 16*3200 = 51200 bytes

//#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - offsetof(type,member) );})

typedef struct __line_pool{
	struct __line_pool * next;
	line_t line;
}line_pool_t;

static line_pool_t *pool_free_list;
static line_pool_t *line_pool;

static void (*_lock)();
static void (*_unlock)();

void line_init(void *ptr,void (*lock)(),void (*unlock)())
{
	_lock = lock;
	_unlock = unlock;

	line_pool = ptr;
	pool_free_list = line_pool;

	for (int i = 1; i < LINE_POOL_SIZE; ++i) {
		pool_free_list->next = &line_pool[i];
		pool_free_list = pool_free_list->next;
	}

	pool_free_list->next = NULL;

	pool_free_list = line_pool;
}

line_t *getLine(void)
{
	line_t *l = NULL;

	_lock();
	if(pool_free_list){
		l = &pool_free_list->line;
		pool_free_list = pool_free_list->next;
	}
	_unlock();

	return l;
}

void freeLine(void *l)
{
	line_pool_t *p = container_of(l,line_pool_t,line);

	_lock();
	p->next = pool_free_list;
	pool_free_list = p;
	_unlock();

}
