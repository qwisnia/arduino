/********************************************************************************
 *
 * Copyright (c) 2016 Krzysztof Wisniewski
 *
 *        ALL RIGHTS RESERVED
 *
 ********************************************************************************
 *
 * Filename       : buffer.c
 * Project        :
 *
 * Description    :
 * Author         : Krzysztof Wisniewski
 * Created        :
 * Last Modified  :
 * Version        :
 *
 *******************************************************************************/
#include "buffer.h"

// @file buffer.c
// @brief Initialize the buffer
//
// @param buffer - the physical memory address where the buffer is stored, void type
// @param size - the allocated size of the buffer, number of data elements which could be stored in the buffer
// @param dataTypeSize - the size of the single element stored in the buffer
// @param type - buffer type
//
// @return bufferStatus_t - Buffer return status
bufferStatus_t buffer_init(
   buffer_t**     buffer,
   unsigned short size,
   unsigned short dataTypeSize,
   bufferType_t   type
) {

   // check if the pointer to the buffer structure exists
   // if not allocate memory for it
   if (*buffer == NULL) {
      *buffer = (buffer_t *)malloc(sizeof(buffer_t));
      if (*buffer == NULL) {
         return BUFFER_FAIL;
      }
   }

   // allocate memory for the buffer
   (*buffer)->dataPtr = calloc(size, dataTypeSize);
   if ((*buffer)->dataPtr == NULL) {
      return BUFFER_FAIL;
   }

   (*buffer)->size         = size;
   (*buffer)->dataTypeSize = dataTypeSize;
   (*buffer)->dataCnt      = 0;
   (*buffer)->wrPtr        = (*buffer)->dataPtr;
   (*buffer)->rdPtr        = NULL;
   (*buffer)->overflowCnt  = 0;
   (*buffer)->type         = type;

   return BUFFER_SUCCESS;
}

// @file buffer.c
// @brief Remove the buffer data and free memory
//
// @param buffer
//
// @return
bufferStatus_t buffer_free(
   buffer_t** buffer
) {

   if ((*buffer == NULL) ||
       ((*buffer)->dataPtr == NULL)) {
      return BUFFER_FAIL;
   }

   free((*buffer)->dataPtr);
   (*buffer)->dataPtr = NULL;

   free(*buffer);
   *buffer = NULL;

   return BUFFER_SUCCESS;
}

// @file buffer.c
// @brief
//
// @param buffer
// @param ptr
//
// @return none
static void _buffer_checkPtrMaxBoundary(
   buffer_t* buffer,
   void**    ptr
) {

   // if ptr went above a maximal allocated memory address,
	// then move it back to the lowest available address in a buffer
   if (*ptr == ((unsigned char *)buffer->dataPtr + (buffer->size * buffer->dataTypeSize))) {
      *ptr = buffer->dataPtr;
   }

}

// @file buffer.c
// @brief
//
// @para buffer
// @param ptr
//
// @return none
static void _buffer_checkPtrMinBoundary(
   buffer_t* buffer,
   void**    ptr
) {

   // if ptr went below a minimal allocated memory address,
	// then move it to the highest available address in a buffer
   if (*ptr < buffer->dataPtr) {
      *ptr = (unsigned char *)buffer->dataPtr + ((buffer->size - 1) * buffer->dataTypeSize);
   }

}

// @file buffer.c
// @brief Put data into a buffer
//
// @param buffer
// @param data
//
// @return BUFFER_STATUS
bufferStatus_t buffer_push(
   buffer_t* buffer,
   void*     data
) {

   // check if a pointed was provided
   if (data == NULL) {
      return BUFFER_PTR_ERROR;
   }

   // check if a pointed was provided
   if (buffer->wrPtr == NULL) {
      return BUFFER_PTR_ERROR;
   }

   switch (buffer->type) {
      #ifdef BUFFER_CIRCULAR_EN
      case BUFFER_CIRCULAR:

         // copy a new element into the buffers memory
         memcpy(
            (unsigned char*)buffer->wrPtr,
            (unsigned char*)data,
            buffer->dataTypeSize
         );

			if (buffer->dataCnt < buffer->size) {
	         buffer->dataCnt++;
			} else {
				buffer->overflowCnt++;
			}

			buffer->rdPtr = buffer->wrPtr;

         break;
      #endif // BUFFER_CIRCULAR_EN
      #ifdef BUFFER_FIFO_EN
      case BUFFER_FIFO:

         // check if the buffer is full
         if (buffer->dataCnt == buffer->size) {
				buffer->overflowCnt++;
            return BUFFER_FULL;
         }

         // copy a new element into the buffers memory
         memcpy(
            (unsigned char*)buffer->wrPtr,
            (unsigned char*)data,
            buffer->dataTypeSize
         );

         buffer->dataCnt++;

         if (buffer->rdPtr == NULL) {
            buffer->rdPtr = buffer->wrPtr;
         }

         break;
      #endif // BUFFER_FIFO_EN
      #ifdef BUFFER_LIFO_EN
      case BUFFER_LIFO:

         // check if the buffer is full
         if (buffer->dataCnt == buffer->size) {
				buffer->overflowCnt++;
            return BUFFER_FULL;
         }

         // copy a new element into the buffers memory
         memcpy(
            (unsigned char*)buffer->wrPtr,
            (unsigned char*)data,
            buffer->dataTypeSize
         );

         buffer->dataCnt++;

         buffer->rdPtr = buffer->wrPtr;

         break;
      #endif // BUFFER_LIFO_EN
      default:
         return BUFFER_TYPE_UNKNOWN;
   }

	buffer->wrPtr = (unsigned char *)buffer->wrPtr + buffer->dataTypeSize;
	_buffer_checkPtrMaxBoundary(buffer, &(buffer->wrPtr));

   return BUFFER_SUCCESS;
}

// @file buffer.c
// @brief Get data out of a buffer
//
// @param buffer
// @param data
//
// @return BUFFER_STATUS
bufferStatus_t buffer_pop(
   buffer_t* buffer,
   void*     data
) {

   if (data == NULL) {
      return BUFFER_PTR_ERROR;
   }

   if ((buffer->rdPtr == NULL) ||
       (buffer->dataCnt == 0)) {
      return BUFFER_EMPTY;
   }

   // copy a new element into the buffers memory
   memcpy(
      (unsigned char*)data,
      (unsigned char*)buffer->rdPtr,
      buffer->dataTypeSize
   );

   buffer->dataCnt--;

   switch (buffer->type) {
		#ifdef BUFFER_LIFO_EN
		case BUFFER_LIFO:
		#endif // BUFFER_LIFO_EN
      #ifdef BUFFER_CIRCULAR_EN
      case BUFFER_CIRCULAR:

         buffer->rdPtr = (unsigned char *)buffer->rdPtr - buffer->dataTypeSize;
         _buffer_checkPtrMinBoundary(buffer, &(buffer->rdPtr));

         buffer->wrPtr = (unsigned char *)buffer->wrPtr - buffer->dataTypeSize;
         _buffer_checkPtrMinBoundary(buffer, &(buffer->wrPtr));

         break;
      #endif // BUFFER_CIRCULAR_EN
      #ifdef BUFFER_FIFO_EN
      case BUFFER_FIFO:

         buffer->rdPtr = (unsigned char *)buffer->rdPtr + buffer->dataTypeSize;
         _buffer_checkPtrMinBoundary(buffer, &(buffer->rdPtr));

         break;
      #endif // BUFFER_FIFO_EN

      default:
         return BUFFER_TYPE_UNKNOWN;
   }

   return BUFFER_SUCCESS;
}

// @file buffer.c
// @brief Empty the buffer, by ignoring all the values
//
// @param buffer
//
// @return BUFFER_STATUS
bufferStatus_t buffer_flush(
   buffer_t* buffer
) {

   buffer->wrPtr       = buffer->dataPtr;
   buffer->rdPtr       = NULL;
   buffer->dataCnt     = 0;
   buffer->overflowCnt = 0;

   return BUFFER_SUCCESS;
}
