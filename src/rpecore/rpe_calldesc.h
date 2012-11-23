
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
 *  @file       rpe_calldesc.h
 *
 *  @brief      Defines RPE call descriptor structures - used by both client
 *              and server sides.
 * 
 */

#ifndef RPE_CALLDESC_H
#define RPE_CALLDESC_H

#include <stdint.h>

/**
 *  @brief      Enumerates types of RPE call.
 */
typedef enum Rpe_CmdType {
    RPE_CMD_CREATE,             /**< Instance create call */
    RPE_CMD_DELETE,             /**< Instance delete call */
    RPE_CMD_CONTROL,            /**< Control call */
    RPE_CMD_PROCESS,            /**< Process call */
    RPE_CMD_GETCONFIG,          /**< Get an RPE config */
    RPE_CMD_SHUTDOWN            /**< Close RPE root server on remote processor */
} Rpe_CmdType;

/**
 *  @brief      Enumerates current state of a call descriptor.
 */
typedef enum Rpe_CallDescStateType {
    RPE_DESC_STATE_FREE,         /**< Not in use */
    RPE_DESC_STATE_ACQUIRED,     /**< Client has acquired it */
    RPE_DESC_STATE_BUSY          /**< Client has made call to the server */
} Rpe_CallDescState;

/**
 *  @brief      Defines base structure for call descriptors.
 *              It is a message buffer allocated from shared memory.
 */
struct Rpe_CallDesc {

    /** SysLink message header */
    MessageQ_MsgHeader          msgHdr;
    
    /** Client handle for an RPE instance */
    Rpe_ClientHandle            clientHandle;
    
    /** Type of call descriptor - CONTROL or PROCESS */ 
    uint8_t                     callDescType;
    
    /** Type of RPE command. It takes vales defined by Rpe_CmdType */
    uint8_t                     cmdType;
    
    /** Current state of the descriptor. It takes vales defined by 
     *  Rpe_CallDescState
     */
    uint8_t                     descState;
    
    /** Number of arguments to the RPE call */
    uint8_t                     argCnt;
    
    /** Handle to the RPE instance object on server side */
    void                       *serverHandle;
    
    /** Return status of an RPE call to the server */
    int32_t                     status;
    
    /** Maximum sizes for the RPE call arguments
     *
     *  @remarks    As call descriptors are allocated when an RPE instance is
     *              created, it is necessary to account for maximum possible 
     *              sizes for the arguments
     */
    uint16_t                    argSize[RPE_MAX_ARGCNT];
};

/**
 *  @brief      Defines extended call descriptor structure for a create call.
 */
typedef struct Rpe_CreateCallDesc {

    /** Base call descriptor structure */
    Rpe_CallDesc                callDesc;
    
    /** Name of the RPE component instance */
    char                        name[RPE_MAX_COMP_NAME_LEN + 1];
    
    /** RPE instance attribute staructure passed by the client */
    Rpe_Attributes              instAttr;
    
    /** Message queue to send all calls to server */
    MessageQ_QueueId            serverMsgqId;

} Rpe_CreateCallDesc;

/**
 *  @brief      Defines extended call descriptor structure for a GetConfig call.
 */
typedef struct Rpe_GetConfigCallDesc {

    /** Base call descriptor structure */
    Rpe_CallDesc                callDesc;
    
    /** Index of the algorithm config. - provided by the client */
    uint16_t                    configIndex;
    
    /** Number of configs available on server */
    uint16_t                    configCount;
    
    /** Name of the RPE component instance */
    char                        name[RPE_MAX_COMP_NAME_LEN + 1];
    
    /** RPE Implementation Class */
    uint32_t                    classId;

    /** If remote, id of the processor where it is located */
    uint8_t                     processorId;

    /** Number of arguments taken by a Control call. */
    uint8_t                     numControlArgs;

    /** Number of arguments taken by a Process call. */    
    uint8_t                     numProcessArgs;

    /** Size of the parameter structure for algorithm create call. */
    uint16_t                    sizeofCreateArgs;
    
    /** Sizes of the arguments to Control call. */
    uint16_t                    sizeofControlArgs[RPE_MAX_ARGCNT]; 
    
    /** Sizes of the arguments to Process call. */
    uint16_t                    sizeofProcessArgs[RPE_MAX_ARGCNT];

} Rpe_GetConfigCallDesc;


/** Start of all call arguments are aligned to this size */
#define RPE_REQ_BUF_STRUCT_ALIGNMENT         4

/** Macro to calculate aligned size of any object */
#define RPE_ALIGNED_SIZE(size) (((size) + (RPE_REQ_BUF_STRUCT_ALIGNMENT - 1)) \
                                        & ~(RPE_REQ_BUF_STRUCT_ALIGNMENT - 1))
/** Macro to calculate address of the first arg to any remote call */
#define RPE_GET_FIRST_ARG_PTR(desc) \
    ((FArg)((char *)(desc) + RPE_ALIGNED_SIZE (sizeof(Rpe_CallDesc))))

#endif     /* RPE_CALLDESC_H */
