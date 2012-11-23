
/*
 *  Copyright (c) 2010-2011, Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  *  Neither the name of Texas Instruments Incorporated nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  Contact information for paper mail:
 *  Texas Instruments
 *  Post Office Box 655303
 *  Dallas, Texas 75265
 *  Contact information:
 *  http://www-k.ext.ti.com/sc/technical-support/product-information-centers.htm?
 *  DCMP=TIHomeTracking&HQS=Other+OT+home_d_contact
 *  ============================================================================
 *
 */

/**
 *  @file       rpe_types.h
 *
 *  @brief      Some basic RPE internal type and macro definitions - Used by 
 *              RPE library functions.
 */

#ifndef RPE_INTERNAL_TYPES_H
#define RPE_INTERNAL_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/**
 *  @brief      Generic opaque pointer - used to represent function call arguments
 *              that are pointers.
 */
typedef void *FArg;

/**
 *  @brief      Generic opaque pointer to any kind of object.
 */
typedef void *Utils_Ptr;

/**
 *  @brief      Generic opaque function pointer. This needs to be converted to one
 *              of Rpe_FxnPtrA1, Rpe_FxnPtrA2, ... Rpe_FxnPtrA5 before making the
 *              actual function call
 */
typedef void *Rpe_FxnPtr;

/*
 *  Forward declaration of type Rpe_CallDesc so that it can be used without
 *  including private header file rpe_calldesc.h.
 */
typedef struct Rpe_CallDesc Rpe_CallDesc;

/** Maximum length of RPE component name */
#define RPE_MAX_COMP_NAME_LEN                31

/** Maximum length of mesasge queue name */
#define RPE_MAX_MQ_NAME_LEN                  31

/** Maximum number of arguments to a RPE control or process call */
#define RPE_MAX_ARGCNT                       6

/** General heap allocation algnment */
#define RPE_MEM_ALLOC_ALIGNMENT              4

/** Root server name format */
#define RPE_ROOT_SERVER_MQ_NAME_FORMAT      "RPE_ROOT_SERVER_MQ_%d"

#ifndef NULL
#define NULL (0)            /**< NULL Pointer */
#endif

#ifndef TRUE
#define TRUE (1)            /**< Boolean TRUE */
#endif

#ifndef FALSE
#define FALSE (0)           /**< Boolean FALSE */
#endif

/** Shared Region Id from where message buffers are allocated */
extern const uint16_t       Rpe_messageqSharedregionId;

/** Heap Id registered with MessageQ for allocating message buffers.
   The heap associated with Rpe_messageqSharedregionId is registered
   as Rpe_messageqHeapId. */
extern const uint16_t       Rpe_messageqHeapId;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* RPE_INTERNAL_TYPES_H */
