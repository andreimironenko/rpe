#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ringbuffer.h"

struct RingBuffer_Object {
    uint32_t        bufferSize;
    uint32_t        lowWaterMark;
    FILE            *fpin;
    uint8_t         *bufferStart;
    uint8_t         eosFlag;
    uint8_t         *curBufPtr;
    uint32_t        bytesInBuf;
};

int32_t RingBuffer_create(RingBuffer_Params  *params, RingBuffer_Object **ringBufferHandle)
{
    RingBuffer_Object   *ringBuffer = NULL;
    
    if ((ringBuffer = (RingBuffer_Object *)malloc(sizeof(RingBuffer_Object))) == NULL)
        return (-1);
    
    ringBuffer->bufferStart = params->buffer;
    ringBuffer->bufferSize = params->bufferSize;
    ringBuffer->lowWaterMark = params->lowWaterMark;
    ringBuffer->fpin = params->fpin;
    ringBuffer->curBufPtr = ringBuffer->bufferStart;
    ringBuffer->bytesInBuf = 0;
    ringBuffer->eosFlag = FALSE;
    
    *ringBufferHandle = ringBuffer;
    
    return (0);
}

int32_t RingBuffer_delete(RingBuffer_Object *rbHandle)
{
    if (NULL == rbHandle)
        return (-1);
    free(rbHandle);
    return (0);
}

int32_t RingBuffer_fill(RingBuffer_Object *rbHandle, uint32_t *bytesInBuf, uint8_t **bufPtr)
{
    uint32_t    bytesToRead;
    uint32_t    bytesRead;
    
    if ((FALSE == rbHandle->eosFlag) && 
        (rbHandle->bytesInBuf < rbHandle->lowWaterMark)) {
        if (rbHandle->bytesInBuf > 0) {
            memmove (rbHandle->bufferStart, rbHandle->curBufPtr, rbHandle->bytesInBuf);
        }
        
        rbHandle->curBufPtr = rbHandle->bufferStart;
            
        bytesToRead = rbHandle->bufferSize - rbHandle->bytesInBuf;
        bytesRead = fread (rbHandle->curBufPtr + rbHandle->bytesInBuf, 1, 
                           bytesToRead, rbHandle->fpin);

        if (bytesRead < bytesToRead)
            rbHandle->eosFlag = TRUE;
            
        if (bytesRead > 0)
            rbHandle->bytesInBuf += bytesRead;
    }
    
    if (rbHandle->bytesInBuf > 0) {
        *bytesInBuf = rbHandle->bytesInBuf;
        *bufPtr = rbHandle->curBufPtr;
        return (TRUE);
    }
    else {
        return (FALSE);
    }
}

int32_t RingBuffer_setBytesConsumed(RingBuffer_Object *rbHandle, uint32_t bytesConsumed)
{
    rbHandle->bytesInBuf -= bytesConsumed;
    rbHandle->curBufPtr += bytesConsumed;
    
    return (0);
}
/*ringio.c - EOF */
