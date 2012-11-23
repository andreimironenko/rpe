
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
 *  @file       rpe.h
 *
 *  @brief      Application Interface Definitions - Used by client 
 *              aplications making call to any algorithm/processing running on
 *              remote processor.
 * 
 */

#ifndef RPE_H
#define RPE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/**
 *  @brief      Opaque handle to Client data structure. 
 */

typedef struct Rpe_ClientObj *Rpe_ClientHandle;

/**
 *  @brief      Opaque pointer to a buffer that contains arguments and other
 *              details about a remote call - create, delete, control, process.
 */
typedef void *Rpe_CallDescHandle;

/**
 *  @brief      Specifies the type of Remote Call Descriptor buffer.
 */
typedef enum Rpe_CallDescType {

    RPE_CALL_DESC_CONTROL = 1,  /**< Control Call Descriptor buffer. Used
                                 *   for making Create, Control, Delete call
                                 */

    RPE_CALL_DESC_PROCESS = 2   /**< Process Call Descriptor buffer
                                 */
} Rpe_CallDescType;

/**
 *  @brief      Specifies priority of a process call. It determines the 
 *              priority of the server thread on the remote processor.
 */
typedef enum Rpe_ProcessingPriority {

    RPE_PROCESSING_PRIORITY_LOW    = 1,   /**< Low priority process call */
    RPE_PROCESSING_PRIORITY_MEDIUM = 2,   /**< Medium priority process call */
    RPE_PROCESSING_PRIORITY_HIGH   = 3,   /**< High priority process call */
    RPE_PROCESSING_PRIORITY_MAX    = 0x7FFFFFFF
} Rpe_ProcessingPriority;

/**
 *  @brief      Specifies how application processor accesses an input/output
 *              buffer.
 *
 *              Based on the access mode, middleware performs cache coherency
 *              operation.
 */
typedef enum Rpe_BufferCpuAccessMode {
    /**
     *  @brief  Cpu access mode is undefined
     */
    RPE_CPU_ACCESS_MODE_UNDEFINED   = 0,

    /**
     *  @brief  Accessed by processor for read. So, cache invalidate
     *          operation is performed before sending a buffer
     *          to a remote processor.
     */
    RPE_CPU_ACCESS_MODE_READ        = 0x01,
    
    /**
     *  @brief  Accessed by processor for write. So, cache writeback-invalidate
     *          operation is performed before sending a buffer
     *          to a remote processor.
     */
    RPE_CPU_ACCESS_MODE_WRITE       = 0x02,
    
    /**
     *  @brief  Not accessed by processor. So, no cache operation
     *          is performed.
     */
    RPE_CPU_ACCESS_MODE_NONE        = 0x03
    
} Rpe_BufferCpuAccessMode;

/**
 *  @brief      Bit-mask to extract access mode of a buffer
 */
#define RPE_CPU_ACCESS_MODE_BITS     0x03

/**
 *  @brief      Macro to set access mode for i th buffer in a set of 
 *              bit-masks. A 32-bit unsigned integer can contain access mode
 *              for 16 buffers.
 */
#define RPE_SET_CPU_ACCESS_MODE(flags,i,mode) \
    (flags |= ((mode) << ((i) << 1)))

/**
 *  @brief      Macro to get access mode for i th buffer from a set of 
 *              bit-masks. A 32-bit unsigned integer can contain access mode
 *              for 16 buffers.
 */
#define RPE_GET_CPU_ACCESS_MODE(flags,i) \
    ((flags >> ((i) << 1)) & RPE_CPU_ACCESS_MODE_BITS)

/**
 *  @brief      Macro to set access mode for all 16 buffers to 
 *              RPE_CPU_ACCESS_MODE_UNDEFINED. 
 */
#define RPE_ALLBUFS_CPU_ACCESS_MODE_UNDEFINED 0x0

/**
 *  @brief      Macro to set access mode for all 16 buffers to 
 *              RPE_CPU_ACCESS_MODE_READ. 
 */
#define RPE_ALLBUFS_CPU_ACCESS_MODE_READ     0x55555555

/**
 *  @brief      Macro to set access mode for all 16 buffers to 
 *              RPE_CPU_ACCESS_MODE_WRITE. 
 */
#define RPE_ALLBUFS_CPU_ACCESS_MODE_WRITE    0xAAAAAAAA

/**
 *  @brief      Macro to set access mode for all 16 buffers to 
 *              RPE_CPU_ACCESS_MODE_NONE. 
 */
#define RPE_ALLBUFS_CPU_ACCESS_MODE_NONE     0xFFFFFFFF

/**
 *  @brief      Specifies attributes for an RPE component instance during a 
 *              Create call.
 */
typedef struct Rpe_Attributes {

    /**
     *  @brief  Processing priority of the RPE component instance.
     *
     *  @sa     Rpe_ProcessingPriority
     */
    Rpe_ProcessingPriority      priority;
    
    /**
     *  @brief  CPU access mode for input buffers on application side. 
     *          Currently, this field can hold access modes for maximum
     *          16 buffers.
     *
     *  @sa     Rpe_BufferCpuAccessMode
     */
    uint32_t                    inBufCpuAccessMode;

    /**
     *  @brief  CPU access mode for output buffers on application side. 
     *          Currently, this field can hold access modes for maximum
     *          16 buffers.
     *
     *  @sa     Rpe_BufferCpuAccessMode
     */
    uint32_t                    outBufCpuAccessMode;

    /**
     *  @brief  CPU access mode for inout buffers on application side. 
     *          Currently, this field can hold access modes for maximum
     *          16 buffers.
     *
     *  @sa     Rpe_BufferCpuAccessMode
     */
    uint32_t                    inOutBufCpuAccessMode;

} Rpe_Attributes;

/** 
 *  @brief      Initializes RPE data structures with the configs available
 *              on the servers. It also initializes the shared region related 
 *              information. Call this function once the firmware is loaded.
 *              It should be called only once before the RPE instances are
 *              created.  
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  Called from: Application
 */
int32_t Rpe_init ();

/** 
 *  @brief      Opaque pointer to parameter structure passed to an algorithm
 *              create call. Application provides this structure to the
 *              Rpe_create() call and it is passed to the algorithm 
 *              create call on the remote processor.
 */
/**
 *  @brief      Create an instance of an RPE component instance.
 *
 *  @param[in]  name            Name of the RPE instance - unique across the 
 *                              syetem.
 *  @param[in]  instAttr        RPE component instance attributes.
 *  @param[in]  createParams    Opaque pointer to parameter structure that is 
 *                              passed to the algorithm create call on the 
 *                              remote processor.
 *  @param[out] clientHandlePtr Opaque handle to the newly created client
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  Called from: Application
 */
int32_t Rpe_create (
    char                       *name,
    Rpe_Attributes             *instAttr,
    void                       *createParams,
    Rpe_ClientHandle           *clientHandlePtr);

/**
 *  @brief      Acquire a Call Descriptor from a client for making control or
 *              process call and initialize application pointers that point to 
 *              the control/process call arguments within a call descriptor.
 *
 *  @param[in]  clientHandle    Client Handle
 *  @param[in]  callDescType    Type of call descriptor - CONTROL or PROCESS
 *  @param[out] descHandle      Opaque handle to the acquired call descriptor.
 *  @param[out] arg1            Pointer to the first argument.
 *  ...
 *  @param[out] argN            Pointer to the Nth argument.
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  Called from: Application
 */
int32_t Rpe_acquireCallDescriptor (
    Rpe_ClientHandle            clientHandle,
    Rpe_CallDescType            callDescType,
    Rpe_CallDescHandle         *descHandle,
                                ...);

/**
 *  @brief      Make a process call to the RPE server.
 *
 *  @param[in]  descHandle      Handle to an already acquired call descriptor
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  Called from: Application
 */
int32_t Rpe_process (Rpe_CallDescHandle descHandle);

/**
 *  @brief      Make a control call to the RPE server.
 *
 *  @param[in]  descHandle      Handle to an already acquired call descriptor
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  Called from: Application
 */
int32_t Rpe_control (Rpe_CallDescHandle descHandle);

/**
 *  @brief      Delete an RPE component instance.
 *
 *  @param[in]  clientHandle    Client Handle
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  Called from: Application
 */
int32_t Rpe_delete (Rpe_ClientHandle clientHandle);

/**
 *  @brief      Shutdown RPE root server on remote processor.
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  Called from: Application
 */
int32_t Rpe_shutdown (uint8_t processorId);

/**
 *  @brief      Free the memory allocated during Rpe_init for the structures
 *              containing information about the configs on the all servers.
 *		This function needs to be called when all the RPE instances have
 *              been executed and the server information is no longer required 
 *              by the client.
 *
 *  @retval     none
 *
 *  Called from: Application
 */
void Rpe_deinit();

/**
 *  @brief      Get the string corresponding to an error code returned by
 *              an API call.
 *
 *  @param[in]  errCode     Error code returned
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  Called from: Application
 */
char *Rpe_getErrorString (int32_t errCode);

/**
 *  @brief      Error codes are common across all modules.
 */
#define RPE_S_SUCCESS                    0

#define RPE_E_BASE                       -1000

#define RPE_E_FAIL                       (RPE_E_BASE - 1)
#define RPE_E_INVALIDARG                 (RPE_E_BASE - 2)
#define RPE_E_NULL_ARGPTR                (RPE_E_BASE - 3)
#define RPE_E_ALREADYEXISTS              (RPE_E_BASE - 4)
#define RPE_E_INVALID_CMD                (RPE_E_BASE - 5)

#define RPE_E_CLIENT_NOT_FOUND           (RPE_E_BASE - 6)
#define RPE_E_INVALID_CLIENT_CLASS_ID    (RPE_E_BASE - 7)
#define RPE_E_INVALID_PROCESSOR_ID       (RPE_E_BASE - 8)
#define RPE_E_INVALID_NUM_CONTROL_ARGS   (RPE_E_BASE - 9)
#define RPE_E_INVALID_CONTROL_ARG_SIZE   (RPE_E_BASE - 10)
#define RPE_E_INVALID_NUM_PROCESS_ARGS   (RPE_E_BASE - 11)
#define RPE_E_INVALID_PROCESS_ARG_SIZE   (RPE_E_BASE - 12)
#define RPE_E_INVALID_RPE_CREATE_PARAM_SIZE (RPE_E_BASE - 13)
#define RPE_E_NULL_CLIENT_MARSHALL_FXN   (RPE_E_BASE - 14)
#define RPE_E_NULL_CLIENT_UNMARSHALL_FXN (RPE_E_BASE - 15)

#define RPE_E_CLIENT_NO_MEMORY           (RPE_E_BASE - 16)
#define RPE_E_CLIENTDESC_NO_MEMORY       (RPE_E_BASE - 17)
#define RPE_E_CLIENTDESC_NOTFREE         (RPE_E_BASE - 18)

#define RPE_E_IPC_READ_CONN              (RPE_E_BASE - 21)
#define RPE_E_IPC_WRITE_CONN             (RPE_E_BASE - 22)
#define RPE_E_IPC_SEND                   (RPE_E_BASE - 23)
#define RPE_E_IPC_RECV                   (RPE_E_BASE - 24)
#define RPE_E_IPC_MSG                    (RPE_E_BASE - 25)

#define RPE_E_SHREG_NOTINIT              (RPE_E_BASE - 31)
#define RPE_E_SHREG_INVALID              (RPE_E_BASE - 32)

#define RPE_E_SERVER_IPC_CONN            (RPE_E_BASE - 41)
#define RPE_E_SERVER_OS                  (RPE_E_BASE - 42)
#define RPE_E_OS_TASK_CREATE             (RPE_E_BASE - 43)
#define RPE_E_OS_SEM_CREATE              (RPE_E_BASE - 44)

#define RPE_E_SERVER_CREATE              (RPE_E_BASE - 51)
#define RPE_E_SERVER_CONTROL             (RPE_E_BASE - 52)
#define RPE_E_SERVER_PROCESS             (RPE_E_BASE - 53)
#define RPE_E_SERVER_DELETE              (RPE_E_BASE - 54)

#define RPE_E_SERVER_NOT_FOUND           (RPE_E_BASE - 61)
#define RPE_E_SERVER_NO_MEMORY           (RPE_E_BASE - 62)
#define RPE_E_INVALID_CONFIG_INDEX       (RPE_E_BASE - 63)
#define RPE_E_NULL_CREATE_FXN            (RPE_E_BASE - 64)
#define RPE_E_NULL_DELETE_FXN            (RPE_E_BASE - 65)
#define RPE_E_NULL_CONTROL_FXN           (RPE_E_BASE - 66)
#define RPE_E_NULL_PROCESS_FXN           (RPE_E_BASE - 67)
#define RPE_E_NULL_SERVER_FXN            (RPE_E_BASE - 68)
#define RPE_E_INVALID_TASK_STACK_SIZE    (RPE_E_BASE - 69)
#define RPE_E_INVALID_SERVER_HANDLE_SIZE (RPE_E_BASE - 70)
#define RPE_E_INVALID_SERVER_CLASS_ID    (RPE_E_BASE - 71)
#define RPE_E_SERVER_BUF_ACCESS_UNDEFINED (RPE_E_BASE - 72)

#define RPE_TOO_MANY_ARGS                (RPE_E_BASE - 81)

#define RPE_E_XDM_NULL_XDMFXNS           (RPE_E_BASE - 101)
#define RPE_E_XDM_NULL_ALGFXNS           (RPE_E_BASE - 102)
#define RPE_E_XDM_NULL_ALGALLOC_FXN      (RPE_E_BASE - 103)
#define RPE_E_XDM_NULL_ALGFREE_FXN       (RPE_E_BASE - 104)
#define RPE_E_XDM_NULL_ALGINIT_FXN       (RPE_E_BASE - 105)
#define RPE_E_XDM_INVALID_CREAT_PARAMS   (RPE_E_BASE - 106)
#define RPE_E_XDM_ALGINIT                (RPE_E_BASE - 107)
#define RPE_E_XDM_NULL_ALG               (RPE_E_BASE - 108)
#define RPE_E_XDAIS_NO_MEMORY            (RPE_E_BASE - 109)
#define RPE_E_XDM_NULL_PROCESS_PTR       (RPE_E_BASE - 110)
#define RPE_E_XDM_NULL_PROCESS           (RPE_E_BASE - 111)
#define RPE_E_XDM_NULL_CONTROL_PTR       (RPE_E_BASE - 112)
#define RPE_E_XDM_NULL_CONTROL           (RPE_E_BASE - 113)

#define RPE_E_UTILS_NO_MEMORY            (RPE_E_BASE - 121)

#define RPE_E_UTILS_TASK_CREATE          (RPE_E_BASE - 131)
#define RPE_E_UTILS_SEM_CREATE           (RPE_E_BASE - 132)
#define RPE_E_UTILS_SEM_PEND             (RPE_E_BASE - 133)
#define RPE_E_UTILS_ERR_UNKNOWN          (RPE_E_BASE - 139)

#define RPE_E_MAXERR                     (RPE_E_BASE - 200)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RPE_H */

