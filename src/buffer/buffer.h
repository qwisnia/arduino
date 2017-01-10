/********************************************************************************
 *
 * Copyright (c) 2016 Krzysztof Wisniewski
 *
 *        ALL RIGHTS RESERVED
 *
 ********************************************************************************
 *
 * Filename       : buffer.h
 * Project        : Generic buffer implementation
 *
 * Description    :
 * Author         : Krzysztof Wisniewski
 * Created        :
 * Last Modified  :
 * Version        :
 ******************************************************************/
#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include <string.h>

// Enable different types of buffers.
// Do that only if you really need them.
#define BUFFER_CIRCULAR_EN
#define BUFFER_FIFO_EN
#define BUFFER_LIFO_EN

// return codes
typedef enum {
   BUFFER_SUCCESS = 0,
   BUFFER_FAIL,
   BUFFER_FULL,
	BUFFER_EMPTY,
   BUFFER_TYPE_UNKNOWN,
   BUFFER_PTR_ERROR
} bufferStatus_t;

// buffer types
typedef enum {
	#ifdef BUFFER_FIFO_EN
   BUFFER_FIFO     = 0,
	#endif

	#ifdef BUFFER_LIFO_EN
   BUFFER_LIFO     = 1,
	#endif

	#ifdef BUFFER_CIRCULAR_EN
   BUFFER_CIRCULAR = 2
	#endif
} bufferType_t;

/* buffer definition */
typedef struct buffer_e {
   void*          dataPtr;      // the physical memory address where the buffer is stored, void type
   unsigned short size;         // the allocated size of the buffer, number of data elements which could be stored in the buffer
   unsigned short dataTypeSize; // the size of the single element stored in the buffer
   unsigned short dataCnt;      // the number of data elements currently stored in the buffer
   void*          wrPtr;
   void*          rdPtr;
   unsigned short overflowCnt;  // a counter indicating how many of the data elements have been lost due to overflow
   bufferType_t   type;         // buffer type
} buffer_t;

bufferStatus_t buffer_init(buffer_t**     buffer,
                           unsigned short size,
                           unsigned short dataTypeSize,
                           bufferType_t   type);
bufferStatus_t buffer_free(buffer_t** buffer);
bufferStatus_t buffer_flush(buffer_t* buffer);
bufferStatus_t buffer_push(buffer_t* buffer,
                           void*     data);
bufferStatus_t buffer_pop(buffer_t* buffer,
                          void*     data);
#define buffer_remove(bufferPtr) buffer_pop(bufferPtr, NULL)

#endif // BUFFER_H