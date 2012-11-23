
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
 *  @file       rpe_client.h
 *
 *  @brief      Defines data structures used by the RPE Client.
 * 
 */

#ifndef RPE_CLIENT_H
#define RPE_CLIENT_H

#include <ti/ipc/MessageQ.h>

#include "ti/rpe.h"
#include "ti/rpe_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 *  @brief      Defines structure for the client side configuration of a
 *              particular RPE implementation class.
 */
typedef struct Rpe_ClientClassConfig {

    /** RPE Implementation Class */
    uint32_t                    classId;

    /** Pointer to the marshalling function for process call. */
    Rpe_FxnPtr                  marshallProcessArgsFxn;

    /** Pointer to the unmarshalling function for process call. */
    Rpe_FxnPtr                  unmarshallProcessArgsFxn;

    /** Pointer to the marshalling function for control call. */
    Rpe_FxnPtr                  marshallControlArgsFxn;

    /** Pointer to the unmarshalling function for control call. */
    Rpe_FxnPtr                  unmarshallControlArgsFxn;

} Rpe_ClientClassConfig;

/**
 *  @brief      Defines structure for the client side configuration of a
 *              particular RPE implementation.
 */
typedef struct Rpe_ClientConfig {

    /** Name of the RPE instance. Used in RPE_create call. */
    char                       *name;

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

} Rpe_ClientConfig;

/**
 *  @brief      Defines structure for the client object. It is allocated in
 *              Rpe_create call.
 */
typedef struct Rpe_ClientObj {

    /** Pointer to the client class config. structure */
    const Rpe_ClientClassConfig *clientClassConfig; 

    /** Pointer to the client config. structure.  */
    Rpe_ClientConfig           *clientConfig; 

    /** Copy of RPE instance attribute structure passed in RPE_create call. */
    Rpe_Attributes              instAttr;

    /** Pointer to the control call descriptor. */
    void                       *controlCallDesc;

    /** Pointer to the process call descriptor. */
    void                       *processCallDesc;

    /** Handle to the RPE instance object on server side. It is initialized 
     *  in create call and passed in subsequent control, process and delete 
     * calls.
     */
    void                       *serverHandle;

    /** MessageQ to send msg to the root server. Used for create call. */
    MessageQ_QueueId            rootServerMsgqId;

    /** MessageQ to send msg to the server thread. 
      * Used for control, process and delete calls. It is same as 
      * rootServerMsgqId if the RPE instance server does not have a separate
      * thread (server runs in the context of the root server).
      */
    MessageQ_QueueId            serverMsgqId;

    /** MessageQ to receive msg in the client. */
    MessageQ_Handle             clientRecvMsgqHndl;

} Rpe_ClientObj;

/**
 *  @brief      Array of client side configuration structures for currently 
 *              defined classes.
 */
extern const Rpe_ClientClassConfig Rpe_clientClassConfigArray[];



#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* RPE_CLIENT_H */
