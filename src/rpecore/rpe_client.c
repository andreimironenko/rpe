
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
 *  @file       rpe_client.c
 *
 *  @brief      Implements RPE core framework's client side functions.
 * 
 */

#define DEBUG_RPE_API 1

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>

#include <ti/Std.h>
#include <ti/syslink/utils/IHeap.h>
#include <ti/ipc/MultiProc.h>

#include "ti/rpe_client.h"
#include "ti/system_utils.h"

#include "rpe_calldesc.h"
#include "rpe_fxncall.h"

/*===================== Static Variables ====================================*/
                        
/**
 *  @brief      Counter to keep track how many times initialization is called.
 */
static int32_t              RpeClient_initializationCheck = 0;

/**
 *  @brief      2D Array of client side configuration structures for all
 *              servers.
 */
Rpe_ClientConfig  **Rpe_clientConfigArray = NULL;

/**
 *  @brief      Total number of processors present in this platform.
 */
uint16_t            Rpe_numProcs;

/**
 *  @brief      Array to store the number of configurations on each server
 */
uint32_t            *Rpe_configCountArray = NULL;

/*===================== Static Function Declarations ========================*/

static int32_t Rpe_sendCallToServer (
    MessageQ_QueueId            serverMsgqId,
    MessageQ_Handle             clientRecvMsgqHndl,
    Rpe_CallDesc               *desc);
    
static void Rpe_generateClientRecvMqName (
    char                        name[]);

static int32_t RpeClient_findClientClassConfig (
    uint32_t                    classId,
    const Rpe_ClientClassConfig **clientClassCfgRet);

static int32_t RpeClient_findClientConfig (
    char *                      name,
    Rpe_ClientConfig          **clientCfgRet);

static int32_t RpeClient_initConfigDB ();

static int32_t Rpe_checkClientClassConfig (
    const Rpe_ClientClassConfig *clientClassCfg);

/*===================== Function Definitions ================================*/

/*****************************************************************************
 *  ========== Rpe_init () ==========
 *
 *  See rpe.h
 *****************************************************************************/

int32_t Rpe_init ()
{
    int32_t                     status = RPE_S_SUCCESS;
    
    if (RpeClient_initializationCheck == 0) {
        /* initialize shared region related information */
        Utils_initSharedRegionAddressTable ();
        
        if ((status = RpeClient_initConfigDB ()) != RPE_S_SUCCESS)
            goto Exit;
        
        RpeClient_initializationCheck++;
    }

Exit:
    return status;
}

/******************************************************************************
 *  ========== Rpe_create () ==========
 *
 *  See rpe.h
 *****************************************************************************/

int32_t Rpe_create (
    char                       *name,
    Rpe_Attributes             *instAttr,
    void                       *createParams,
    Rpe_ClientHandle           *clientHandlePtr)
{
    int32_t                     status = RPE_S_SUCCESS;
    Rpe_ClientConfig           *clientCfg = NULL;;
    const Rpe_ClientClassConfig  *clientClassCfg = NULL;
    Rpe_ClientObj              *clientObj = NULL;
    char                        rootServerMqName[RPE_MAX_MQ_NAME_LEN + 1];
    uint16_t                    createDescSize;
    uint16_t                    controlDescSize;
    uint16_t                    processDescSize;
    Rpe_CreateCallDesc         *createCallDesc = NULL;
    Rpe_CallDesc               *desc = NULL;
    char                        recvMqName[RPE_MAX_MQ_NAME_LEN + 1];
    uint16_t                    i;
    uint16_t                    argSize;

    if ((NULL == name) || (NULL == clientHandlePtr)) {
        status = RPE_E_INVALIDARG;
        goto Error;
    }

    *clientHandlePtr = NULL;

    /* Find configuration records for the client */
    if ((status = RpeClient_findClientConfig (name, &clientCfg)) 
                                                        != RPE_S_SUCCESS)
        goto Error;

    if ((status = RpeClient_findClientClassConfig (clientCfg->classId, 
                                          &clientClassCfg)) != RPE_S_SUCCESS)
        goto Error; 

    if ((status = Rpe_checkClientClassConfig (clientClassCfg)) != RPE_S_SUCCESS)
        goto Error;

    /* Allocate memory for client object */
    if ((clientObj = (Rpe_ClientObj *) malloc (sizeof (Rpe_ClientObj))) == NULL) {
        status = RPE_E_CLIENT_NO_MEMORY;
        goto Error;
    }

    /* Initialize fields in client object */
    clientObj->clientConfig = clientCfg;
    clientObj->clientClassConfig = clientClassCfg;
    clientObj->controlCallDesc = clientObj->processCallDesc = NULL;
    clientObj->instAttr = *instAttr;

    clientObj->rootServerMsgqId = clientObj->serverMsgqId
                                = MessageQ_INVALIDMESSAGEQ;
    clientObj->clientRecvMsgqHndl = NULL;

    /* 
     * Open connection to the root server. 
     */
    snprintf (rootServerMqName, RPE_MAX_MQ_NAME_LEN + 1, 
              RPE_ROOT_SERVER_MQ_NAME_FORMAT, clientCfg->processorId);
    if (MessageQ_open (rootServerMqName, &clientObj->rootServerMsgqId)
                                               != MessageQ_S_SUCCESS) {
        status = RPE_E_IPC_WRITE_CONN;
        goto Error;
    }
    clientObj->serverMsgqId = clientObj->rootServerMsgqId;

    /* Create message queue to receive reply */
    Rpe_generateClientRecvMqName (recvMqName);
    if ((clientObj->clientRecvMsgqHndl = MessageQ_create (recvMqName, NULL))
                                                                     == NULL) {
        status = RPE_E_IPC_READ_CONN;
        goto Error;
    }

    /* 
     * Find sizes of create and control call desc. buffers. As the same buffer
     * is used for both create and control calls, the size of the allocated
     * buffer is the max of 'createDescSize' and 'controlDescSize'.
     */
    createDescSize = (RPE_ALIGNED_SIZE (sizeof (Rpe_CreateCallDesc))
                        + RPE_ALIGNED_SIZE (clientCfg->sizeofCreateArgs));
    
    for (controlDescSize = RPE_ALIGNED_SIZE (sizeof (Rpe_CallDesc)), i = 0;
         i < clientCfg->numControlArgs; i++)
         controlDescSize += RPE_ALIGNED_SIZE (clientCfg->sizeofControlArgs[i]);

    if (controlDescSize < createDescSize)
        controlDescSize = createDescSize;

    /* Allocate buffer for create/control call */
    if ((desc = (Rpe_CallDesc *) MessageQ_alloc (Rpe_messageqHeapId,
                                                 controlDescSize)) == NULL) {
        status = RPE_E_CLIENTDESC_NO_MEMORY;
        goto Error;
    }

    /* 
     * The descriptor is used first to make a create call and 
     * to make all control calls subsequently.
     */
    createCallDesc = (Rpe_CreateCallDesc *) desc;
    clientObj->controlCallDesc = desc;

    /* Fill up fields in the descriptor for a create call */
    desc->clientHandle = clientObj;
    desc->callDescType = RPE_CALL_DESC_CONTROL;
    desc->cmdType = RPE_CMD_CREATE;
    desc->descState = RPE_DESC_STATE_FREE;
    desc->serverHandle = NULL;
    desc->status = RPE_E_SERVER_CREATE;

    MessageQ_setReplyQueue (clientObj->clientRecvMsgqHndl,
                            (MessageQ_Msg) (desc));

    /* 
     * 3 arguments are passed to create call - 1) name, 2) RPE component 
     * instance attributes and 3) Algorithm create param
     */
    desc->argCnt = 3;

    desc->argSize[0] = argSize = RPE_ALIGNED_SIZE (RPE_MAX_COMP_NAME_LEN + 1);
    memcpy (createCallDesc->name, clientCfg->name, argSize);

    desc->argSize[1] = RPE_ALIGNED_SIZE (sizeof (Rpe_Attributes));
    createCallDesc->instAttr = *instAttr;

    desc->argSize[2] = argSize = RPE_ALIGNED_SIZE (clientCfg->sizeofCreateArgs);
    memcpy (((char *)(createCallDesc)
             + RPE_ALIGNED_SIZE (sizeof (Rpe_CreateCallDesc))),
            createParams, argSize);

    /* Make a blocking call to the root server */
    if ((status = Rpe_sendCallToServer (clientObj->serverMsgqId,
                                        clientObj->clientRecvMsgqHndl,
                                        (Rpe_CallDesc *) createCallDesc)) 
                                                         != RPE_S_SUCCESS)
        goto Error;

    /* Save serverHandle and serverMsgqId returned from create call */
    clientObj->serverHandle = createCallDesc->callDesc.serverHandle;
    clientObj->serverMsgqId = createCallDesc->serverMsgqId;

    /* Now intialize fields in the descriptor for future control calls */
    desc->cmdType       = RPE_CMD_CONTROL;
    desc->descState     = RPE_DESC_STATE_FREE;
    desc->status        = RPE_E_SERVER_CONTROL;

    desc->argCnt        = clientCfg->numControlArgs;
    for (i = 0; i < clientCfg->numControlArgs; i++)
        desc->argSize[i] = RPE_ALIGNED_SIZE (clientCfg->sizeofControlArgs[i]);

    /* Calculate total size of process desc. */
    for (processDescSize = RPE_ALIGNED_SIZE (sizeof (Rpe_CallDesc)), i = 0; 
         i < clientCfg->numProcessArgs; i++)
         processDescSize += RPE_ALIGNED_SIZE (clientCfg->sizeofProcessArgs[i]);


    /* Allocate buffer for process call */
    if ((clientObj->processCallDesc = desc =
         (Rpe_CallDesc *) MessageQ_alloc (Rpe_messageqHeapId,
                                          processDescSize)) == NULL) {
        status = RPE_E_CLIENTDESC_NO_MEMORY;
        goto Error;
    }

    /* Fill up fields in the descriptor for a process call */
    desc->clientHandle  = clientObj;
    desc->callDescType  = RPE_CALL_DESC_PROCESS;
    desc->cmdType       = RPE_CMD_PROCESS;
    desc->descState     = RPE_DESC_STATE_FREE;
    desc->serverHandle  = clientObj->serverHandle;
    desc->status        = RPE_E_SERVER_PROCESS;

    MessageQ_setReplyQueue (clientObj->clientRecvMsgqHndl,
                            (MessageQ_Msg) (desc));

    desc->argCnt = clientCfg->numProcessArgs;
    for (i = 0; i < clientCfg->numProcessArgs; i++)
        desc->argSize[i] = RPE_ALIGNED_SIZE (clientCfg->sizeofProcessArgs[i]);

    /* Return handle to the client object */
    *clientHandlePtr = (Rpe_ClientHandle) clientObj;

    goto Exit;

Error:

    /* If error, free up all resources */
    if (NULL != clientObj) {
        if (NULL != clientObj->clientRecvMsgqHndl)
            MessageQ_delete (&clientObj->clientRecvMsgqHndl);

        if (NULL != clientObj->controlCallDesc)
            MessageQ_free ((MessageQ_Msg) (clientObj->controlCallDesc));

        if (NULL != clientObj->processCallDesc)
            MessageQ_free ((MessageQ_Msg) (clientObj->processCallDesc));

        free (clientObj);
    }

Exit:
    return (status);
}

/******************************************************************************
 *  ========== Rpe_delete () ==========
 *
 *  See rpe.h
 *****************************************************************************/

int32_t Rpe_delete (
    Rpe_ClientHandle            clientHandle)
{
    Rpe_ClientObj              *clientObj;
    Rpe_CallDesc               *desc;
    int32_t                     status = RPE_S_SUCCESS;
    
    if (NULL == clientHandle) {
        status = RPE_E_INVALIDARG;
        goto Exit;
    }

    clientObj = (Rpe_ClientObj *) clientHandle;

    /* Use control call descriptor to send the delete call */
    desc = (Rpe_CallDesc *) clientObj->controlCallDesc;

    /* Fill up descriptor fields */
    desc->cmdType = RPE_CMD_DELETE;
    desc->argCnt = 0;
    desc->status = RPE_E_SERVER_DELETE;

    /* Make a blocking call to the server */
    if ((status = Rpe_sendCallToServer (clientObj->serverMsgqId,
                                        clientObj->clientRecvMsgqHndl,
                                        desc)) != RPE_S_SUCCESS)
        goto Exit;

    /* Delete the receiving message queue */
    MessageQ_delete (&clientObj->clientRecvMsgqHndl);

    /* Free up the control and process call descriptors */
    MessageQ_free ((MessageQ_Msg) (clientObj->controlCallDesc));
    MessageQ_free ((MessageQ_Msg) (clientObj->processCallDesc));

    /* Free up client object */
    free (clientHandle);

Exit:
    return (status);
}

/******************************************************************************
 *  ========== Rpe_acquireCallDescripter () ==========
 *
 *  See rpe.h
 *****************************************************************************/

int32_t Rpe_acquireCallDescriptor (
    Rpe_ClientHandle            clientHandle,
    Rpe_CallDescType            callDescType,
    Rpe_CallDescHandle         *descHandle,
                                ...)
{
    Rpe_CallDesc               *desc = NULL;
    Rpe_ClientObj              *clientObj = (Rpe_ClientObj *) clientHandle;
    int32_t                     status = RPE_S_SUCCESS;
    FArg                       *callerArg;
    va_list                     ap;
    FArg                        args[RPE_MAX_ARGCNT];
    uint8_t                     argCnt;
    uint8_t                     i;
    
    if ((NULL == clientHandle) || (NULL == descHandle)) {
        status = RPE_E_INVALIDARG;
        goto Exit;
    }

    if ((RPE_CALL_DESC_CONTROL != callDescType)
        && (RPE_CALL_DESC_PROCESS != callDescType)) {
        status = RPE_E_INVALIDARG;
        goto Exit;
    }

    *descHandle = NULL;

    desc = (RPE_CALL_DESC_CONTROL == callDescType) 
           ? (Rpe_CallDesc *) clientObj->controlCallDesc
           : (Rpe_CallDesc *) clientObj->processCallDesc;

    if (RPE_DESC_STATE_FREE != desc->descState) {       /* Ensure it is free */
        status = RPE_E_CLIENTDESC_NOTFREE;
        goto Exit;
    }

    Rpe_getFunctionCallArgs (desc, &argCnt, args);
    if (argCnt > RPE_MAX_ARGCNT) {
        status = RPE_TOO_MANY_ARGS;
        goto Exit;
    }
    	 
    /* 
     * Caller has passed pointers to the argument pointers.
     * Calculate all the argument pointers and return to the caller.
     */
    va_start (ap, descHandle);

    for (i = 0; i < argCnt; ++i) {

        callerArg = va_arg (ap, void **);

        if (NULL == callerArg)
            break;

        *callerArg = args[i];
    }

    va_end (ap);

    if (i < argCnt) {
        status = RPE_E_NULL_ARGPTR;
        goto Exit;
    }
        
    /* Change state from FREE to ACQUIRED */
    desc->descState = RPE_DESC_STATE_ACQUIRED; 

    /* Return the acquired call descriptor */
    *descHandle = (Rpe_CallDescHandle) desc; 

Exit:
    return (status);
}

/******************************************************************************
 *  ========== Rpe_process () ==========
 *
 *  See rpe.h
 *****************************************************************************/

int32_t Rpe_process (
    Rpe_CallDescHandle descHandle)
{
    int32_t                     status = RPE_S_SUCCESS;
    int32_t                     status1 = RPE_S_SUCCESS;

    Rpe_ClientObj              *clientObj;
    const Rpe_ClientClassConfig *clientClassConfig; 
    Rpe_CallDesc               *desc;
    FArg                        args[RPE_MAX_ARGCNT + 1];
    uint8_t                     argCnt;
    Rpe_FxnPtr                  marshallFxn;
    Rpe_FxnPtr                  unmarshallFxn;

    desc = (Rpe_CallDesc *) descHandle;

    if ((NULL == desc) || (RPE_CALL_DESC_PROCESS != desc->callDescType)) {
        status = RPE_E_INVALIDARG;
        goto Exit;
    }

    clientObj = (Rpe_ClientObj *) (desc->clientHandle);
    clientClassConfig = clientObj->clientClassConfig;

    /*
     * 1) Call marshalling function, if applicable
     * 2) Send call to the server
     * 3) Call unmarshalling function, if applicable
     */

    args[0] = clientObj;
    Rpe_getFunctionCallArgs (desc, &argCnt, &args[1]);

    /* Call marshalling function for process call */
    marshallFxn = clientClassConfig->marshallProcessArgsFxn;
    if (NULL != marshallFxn) {
        if ((status = Rpe_makeFunctionCall (marshallFxn, argCnt + 1, args))
                                                           != RPE_S_SUCCESS) {

            goto Exit;
        }
    }

    /* Make a blocking call to the server */
    status = Rpe_sendCallToServer (clientObj->serverMsgqId,
                                        clientObj->clientRecvMsgqHndl,
                                        desc);

    /* Call unmarshalling function */
    unmarshallFxn = clientClassConfig->unmarshallProcessArgsFxn;
    if (NULL != unmarshallFxn) {
        if ((status1 = Rpe_makeFunctionCall (unmarshallFxn, argCnt + 1, args)
                                                            != RPE_S_SUCCESS)) {
            status = status1;
            goto Exit;
        }
    }

Exit:
    return (status);
}

/******************************************************************************
 *  ========== Rpe_control () ==========
 *
 *  See rpe.h
 *****************************************************************************/

int32_t Rpe_control (
    Rpe_CallDescHandle          descHandle)
{
    int32_t                     status = RPE_S_SUCCESS;
    Rpe_ClientObj              *clientObj;
    const Rpe_ClientClassConfig *clientClassConfig; 
    Rpe_CallDesc               *desc;
    FArg                        args[RPE_MAX_ARGCNT + 1];
    uint8_t                     argCnt;
    Rpe_FxnPtr                  marshallFxn;
    Rpe_FxnPtr                  unmarshallFxn;

    desc = (Rpe_CallDesc *) descHandle; 
    
    if ((NULL == desc) || (RPE_CALL_DESC_CONTROL != desc->callDescType)) {
        status = RPE_E_INVALIDARG;
        goto Exit;
    }

    clientObj = (Rpe_ClientObj *) (desc->clientHandle);
    clientClassConfig = clientObj->clientClassConfig;

    /*
     * 1) Call marshalling function, if applicable
     * 2) Send call to the server
     * 3) Call unmarshalling function, if applicable
     */

    args[0] = clientObj;
    Rpe_getFunctionCallArgs (desc, &argCnt, &args[1]);

    /* Call marshalling function for control call */
    marshallFxn = clientClassConfig->marshallControlArgsFxn;
    if (NULL != marshallFxn) {
        if ((status = Rpe_makeFunctionCall (marshallFxn, argCnt + 1, args)
                                                           != RPE_S_SUCCESS)) {
            goto Exit;
        }
    }

    /* Make a blocking call to the server */
    if ((status = Rpe_sendCallToServer (clientObj->serverMsgqId,
                                        clientObj->clientRecvMsgqHndl,
                                        desc)) != RPE_S_SUCCESS) {
        goto Exit;
    }

    /* Call unmarshalling function */
    unmarshallFxn = clientClassConfig->unmarshallControlArgsFxn;
    if (NULL != unmarshallFxn) {
        if ((status = Rpe_makeFunctionCall (unmarshallFxn, argCnt + 1, args)
                                                            != RPE_S_SUCCESS)) {
            goto Exit;
        }
    }
Exit:
    return (status);
}

/** **************************************************************************
 *  @brief      Generate a messageq name to receive message in client from
 *              server.
 *
 *  @param[out] name            Generated messageq name.
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  Called from: Rpe_create()
 *****************************************************************************/

static void Rpe_generateClientRecvMqName (
    char                        name[])
{
    static uint32_t             curMqInstId = 0;
    uint32_t                    newMqInstId;
    uint16_t                    selfProcessorId;
    pid_t                       selfProcessId;

    /* Get processor and process id for self */
    selfProcessorId = MultiProc_self ();
    selfProcessId = getpid ();

    newMqInstId = curMqInstId++;

    snprintf (name, RPE_MAX_MQ_NAME_LEN + 1, "RPE_CL_MQ_%u_%u_%u", 
             (uint32_t)selfProcessorId, (uint32_t)selfProcessId, newMqInstId);
    return;
}

/** **************************************************************************
 *  @brief      This function sends a RPE call request to the server on
 *              a remote processor and blocks till the reply comes back.
 *
 *  @param[in]  clientObj   Pointer to a client object.
 *  @param[in]  desc        Pointer to a call descriptor.
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  Called from: Rpe_create(), Rpe_delete() and Rpe_call()
 *****************************************************************************/

static int32_t Rpe_sendCallToServer (
    MessageQ_QueueId            serverMsgqId,
    MessageQ_Handle             clientRecvMsgqHndl,
    Rpe_CallDesc               *desc)
{
    MessageQ_Msg                reply;
    
    /* Send message to the server */
    if (MessageQ_put (serverMsgqId, (MessageQ_Msg) desc)
                                                    != MessageQ_S_SUCCESS) {
        desc->status = RPE_E_IPC_SEND;
        goto Exit;
    }

    /* Get the reply */
    if (MessageQ_get (clientRecvMsgqHndl, &reply, MessageQ_FOREVER)
                                                    != MessageQ_S_SUCCESS) {
        desc->status =  RPE_E_IPC_RECV;
        goto Exit;
    }

    /* Ensure that the received message buffer is the same as the one sent */
    if ((MessageQ_Msg) desc != reply)
        desc->status =  RPE_E_IPC_MSG;

Exit:
    return (desc->status);
}

/******************************************************************************
 *  ========== RpeClient_findClientClassConfig () ==========
 *
 *  See rpe_client.h
 *****************************************************************************/

static int32_t RpeClient_findClientClassConfig (
    uint32_t                    classId,
    const Rpe_ClientClassConfig **clientClassCfgRet)
{
    const Rpe_ClientClassConfig *clientClassCfg;
    int32_t                     status;
    
    *clientClassCfgRet = NULL;
    status = RPE_E_INVALID_CLIENT_CLASS_ID;

    for (clientClassCfg = &Rpe_clientClassConfigArray[0];
         0 != clientClassCfg->classId; ++clientClassCfg) {
         
        if (clientClassCfg->classId == classId) {
            *clientClassCfgRet = clientClassCfg;
            status = RPE_S_SUCCESS;
            break;
        }
    }
    
    return (status);
}

/******************************************************************************
 *  ========== RpeClient_findClientConfig () ==========
 *
 *  See rpe_client.h
 *****************************************************************************/

static int32_t RpeClient_findClientConfig (
    char                       *name,
    Rpe_ClientConfig          **clientCfgRet)
{
    uint16_t                    serverProcId;
    Rpe_ClientConfig           *clientConfigArray;
    Rpe_ClientConfig           *clientCfg;
    int32_t                     status;
    
    *clientCfgRet   = NULL;
    status          = RPE_E_CLIENT_NOT_FOUND;
    
    for (serverProcId = 0; serverProcId < Rpe_numProcs; ++serverProcId) {
    
        clientConfigArray = Rpe_clientConfigArray[serverProcId];
        /* skip the search if there is no config defined on the remote processor */
        if (NULL == clientConfigArray) {
             continue;
        }

        for (clientCfg = &clientConfigArray[0];
             NULL != clientCfg->name; ++clientCfg) {
         
            if (0 == strcmp (name, clientCfg->name)) {
                *clientCfgRet = clientCfg;
                status = RPE_S_SUCCESS;
                goto Exit;
            }
        }
    }
Exit:
    return (status);
}

/******************************************************************************
 *  ========== RpeClient_initConfigDB () ==========
 *
 *  See rpe_client.h
 *****************************************************************************/

static int32_t RpeClient_initConfigDB ()
{
    int32_t                     status = RPE_S_SUCCESS;
    uint16_t                    selfProcId;
    uint16_t                    serverProcId;
    char                        rootServerMqName[RPE_MAX_MQ_NAME_LEN + 1];
    MessageQ_QueueId            rootServerMsgqId;
    char                        recvMqName[RPE_MAX_MQ_NAME_LEN + 1];
    MessageQ_Handle             clientRecvMsgqHndl = NULL;
    Rpe_GetConfigCallDesc      *desc = NULL;
    Rpe_ClientConfig           *clientConfigArray = NULL;
    uint16_t                    configCount;    
    uint16_t                    cfgIdx;            /* Config index */
    Rpe_ClientConfig           *cfg;
    uint16_t                    argIdx;            /* Call param index */
    uint32_t                    count;
    
    Rpe_numProcs = MultiProc_getNumProcessors (); /* Total no. of processors */
    selfProcId   = MultiProc_self ();             /* Host's own processor id */
    
    /* Create MessageQ to receive reply - used with all remote processors */
    Rpe_generateClientRecvMqName (recvMqName);
    if ((clientRecvMsgqHndl = MessageQ_create (recvMqName, NULL))
                                                                     == NULL) {
        status = RPE_E_IPC_READ_CONN;
        goto Exit;
    }

    /* Allocate message buffer to make call - used with all remote processors */
    if ((desc = (Rpe_GetConfigCallDesc *) MessageQ_alloc (Rpe_messageqHeapId,
                                                sizeof(Rpe_GetConfigCallDesc)))
                                                                     == NULL) {
        status = RPE_E_CLIENTDESC_NO_MEMORY;
        goto Exit;
    }
    MessageQ_setReplyQueue (clientRecvMsgqHndl, (MessageQ_Msg) (desc));
    
    /* Fill up fields in the descriptor for a Get Config call */
    desc->callDesc.cmdType = RPE_CMD_GETCONFIG;
    
    if ((Rpe_clientConfigArray = (Rpe_ClientConfig  **) 
         malloc (sizeof(Rpe_ClientConfig  *) * Rpe_numProcs)) == NULL) {
        status = RPE_E_CLIENT_NO_MEMORY;
        goto Error;
    }
    
    if ((Rpe_configCountArray = (uint32_t *) 
         malloc (sizeof(uint32_t) * Rpe_numProcs)) == NULL) {
        status = RPE_E_CLIENT_NO_MEMORY;
        goto Error;
    }
    
    for (serverProcId = 0; serverProcId < Rpe_numProcs; ++serverProcId) {

        Rpe_clientConfigArray[serverProcId] = NULL;
        Rpe_configCountArray[serverProcId] = 0;
        
        if (serverProcId == selfProcId) {
            /* If processor id is same as the id of client's (host's) skip */
            continue;
        }

        /* 
         * Open connection to the root server. 
         */
        snprintf (rootServerMqName, RPE_MAX_MQ_NAME_LEN + 1, 
                                RPE_ROOT_SERVER_MQ_NAME_FORMAT, serverProcId);
        if (MessageQ_open (rootServerMqName, &rootServerMsgqId)
                                               != MessageQ_S_SUCCESS) {
            /* If RPE server is not running for this processor, skip */
            continue;
        }

        /* Get all the configs one by one */
        for (cfgIdx = 0; ; ) {
    
            /* Fill up index of the configuration to make a Get Config call */
            desc->configIndex = cfgIdx;

            /* Send request to the server */
            if ((status = Rpe_sendCallToServer (rootServerMsgqId,
                                                clientRecvMsgqHndl,
                                                (Rpe_CallDesc *) desc)) 
                                                         != RPE_S_SUCCESS)
                goto Error;
        
            if ((status = desc->callDesc.status) != RPE_S_SUCCESS)
                goto Error;
            
            if (0 == cfgIdx) {
                /* 
                 * First config from server - allocate memory to store 
                 * (configCount + 1) configs. The last one will have name =  NULL
                 */
                configCount = desc->configCount;
                Rpe_configCountArray[serverProcId] = desc->configCount;
            
                if ((clientConfigArray = (Rpe_ClientConfig *) 
                    malloc (sizeof (Rpe_ClientConfig) * (configCount + 1))) 
                                                            == NULL) {
                    status = RPE_E_CLIENT_NO_MEMORY;
                    goto Error;
                }
                Rpe_clientConfigArray[serverProcId] = clientConfigArray;
            }
        
            /* Copy config info from desc to local config structure */
            cfg = &clientConfigArray[cfgIdx];

            if ((cfg->name = (char *)malloc(strlen(desc->name) + 1)) == NULL) {
                status = RPE_E_CLIENT_NO_MEMORY;
                goto Error;
            }
            strcpy (cfg->name, desc->name);
        
            cfg->classId              = desc->classId;
            cfg->processorId          = desc->processorId;
            cfg->numControlArgs       = desc->numControlArgs;
            cfg->numProcessArgs       = desc->numProcessArgs;
            cfg->sizeofCreateArgs     = desc->sizeofCreateArgs;
        
            for (argIdx = 0; argIdx < RPE_MAX_ARGCNT; argIdx++) {
                cfg->sizeofControlArgs[argIdx] = desc->sizeofControlArgs[argIdx];
                cfg->sizeofProcessArgs[argIdx] = desc->sizeofProcessArgs[argIdx];
            }
        
            if (++cfgIdx == configCount) {
                /* Reached last config, set 'name' to NULL */
                cfg = &clientConfigArray[cfgIdx];
                cfg->name = NULL;
                break;
            }
        }
    }

Error:
    /* If error, free up all resources */
    if (RPE_S_SUCCESS != status) {
        if ((NULL != Rpe_clientConfigArray) && (NULL != Rpe_configCountArray)) {
            for (serverProcId = 0; serverProcId < Rpe_numProcs; ++serverProcId) {
                for (count = 0; 
                count < Rpe_configCountArray[serverProcId]; ++count) {
                    /* free the memory allocated for config name */
                    if (NULL != Rpe_clientConfigArray[serverProcId][count].name)
                        free (Rpe_clientConfigArray[serverProcId][count].name);
                }
        
                /* free the clientConfig structure */ 
                if (NULL != Rpe_clientConfigArray[serverProcId])
                    free (Rpe_clientConfigArray[serverProcId]);
    	    }
    	    /* free the memory for the array for config count */
    	    free (Rpe_configCountArray);
    	}
    	if (NULL != Rpe_clientConfigArray)
 	    free (Rpe_clientConfigArray);
    }

Exit:
    if (NULL != clientRecvMsgqHndl)
        MessageQ_delete (&clientRecvMsgqHndl);

    if (NULL != desc)
        MessageQ_free ((MessageQ_Msg) (desc));

    return (status);
}

/** **************************************************************************
 *  @brief      Check that a client class configuration is correctly 
 *              initialized.
 *
 *  @param[in]  clientClassCfg   Pointer to a client class configuration 
 *                               structure.
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  Called from: Rpe_create()
 *****************************************************************************/

static int32_t Rpe_checkClientClassConfig (
    const Rpe_ClientClassConfig *clientClassCfg)
{
    int32_t         status = RPE_S_SUCCESS;

    if (NULL == clientClassCfg->marshallProcessArgsFxn)
        status =  RPE_E_NULL_CLIENT_MARSHALL_FXN;
    else if (NULL == clientClassCfg->unmarshallProcessArgsFxn)
        status = RPE_E_NULL_CLIENT_UNMARSHALL_FXN;

    return (status);
}


/******************************************************************************
 *  ========== Rpe_shutdown () ==========
 *
 *  See rpe.h
 *****************************************************************************/

int32_t Rpe_shutdown (
    uint8_t                     serverProcId)
{
    int32_t                     status = RPE_S_SUCCESS;
    char                        rootServerMqName[RPE_MAX_MQ_NAME_LEN + 1];
    MessageQ_QueueId            rootServerMsgqId;
    char                        recvMqName[RPE_MAX_MQ_NAME_LEN + 1];
    MessageQ_Handle             clientRecvMsgqHndl = NULL;
    Rpe_CallDesc               *desc = NULL;

    /* Create MessageQ to receive reply - used with all remote processors */
    Rpe_generateClientRecvMqName (recvMqName);
    if ((clientRecvMsgqHndl = MessageQ_create (recvMqName, NULL))
                                   == NULL) {
        status = RPE_E_IPC_READ_CONN;
        goto Exit;
    }

    /* Allocate message buffer to make call - used with all remote processors */
    if ((desc = (Rpe_CallDesc *) MessageQ_alloc (Rpe_messageqHeapId,
                          sizeof(Rpe_CallDesc))) == NULL) {
        status = RPE_E_CLIENTDESC_NO_MEMORY;
        goto Exit;
    }
    MessageQ_setReplyQueue (clientRecvMsgqHndl, (MessageQ_Msg) (desc));

    /* Fill up fields in the descriptor for a Get Config call */
    desc->cmdType = RPE_CMD_SHUTDOWN;

    /* 
     * Open connection to the root server. 
     */
    snprintf (rootServerMqName, RPE_MAX_MQ_NAME_LEN + 1, 
                              RPE_ROOT_SERVER_MQ_NAME_FORMAT, serverProcId);
    if (MessageQ_open (rootServerMqName, &rootServerMsgqId)
            != MessageQ_S_SUCCESS) {
       status = RPE_E_IPC_WRITE_CONN;
       /* If RPE server is not running for this processor, skip */
       goto Exit;
    }

    /* Send request to the server */
    if ((status = Rpe_sendCallToServer (rootServerMsgqId,
                                        clientRecvMsgqHndl,
                                       (Rpe_CallDesc *) desc)) 
                                       != RPE_S_SUCCESS)
        goto Exit;


Exit:
    if (NULL != clientRecvMsgqHndl)
        MessageQ_delete (&clientRecvMsgqHndl);

    if (NULL != desc)
        MessageQ_free ((MessageQ_Msg) (desc));

    return (status);
}

/******************************************************************************
 *  ========== Rpe_deinit () ==========
 *
 *  See rpe.h
 *****************************************************************************/
 
void Rpe_deinit ()
{
    uint16_t                    serverProcId;
    uint32_t                    configCount;
    
    for (serverProcId = 0; serverProcId < Rpe_numProcs; ++serverProcId) {
        for (configCount = 0; 
          configCount < Rpe_configCountArray[serverProcId]; ++configCount) {
            /* free the memory allocated for config name during init */
            if (NULL != Rpe_clientConfigArray[serverProcId][configCount].name)
                free (Rpe_clientConfigArray[serverProcId][configCount].name);
        }
        
        /* free the clientConfig structure */ 
        if (NULL != Rpe_clientConfigArray[serverProcId])
            free (Rpe_clientConfigArray[serverProcId]);
    }
    
    /*
     * free the memory allocated for storing the config count on and the client
     * configuration information
     */
    free (Rpe_configCountArray);
    free (Rpe_clientConfigArray);
}
