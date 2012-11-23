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
 *  @file       rpe_server.c
 *
 *  @brief      Implements RPE core framework's server side functions.
 * 
 */

#include <xdc/runtime/IHeap.h>
#include <ti/ipc/SharedRegion.h>
#include <ti/ipc/MessageQ.h>
#include <ti/ipc/MultiProc.h>

#include <ti/sysbios/heaps/HeapMem.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/Error.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ti/rpe_server.h"
#include "ti/system_utils.h"
#include "ti/rpe_server_task_config.h"

#include "rpe_calldesc.h"
#include "rpe_fxncall.h"

#include "mem_setup.h"

uint8_t  *RPE_dspAlgHeapName;
uint8_t  *RPE_dspAlgHeapHandle;

/*===================== Static Variables ====================================*/

/**
 *  @brief      MessageQ handle to receive messages in the root server.
 */
static MessageQ_Handle      Rpe_rootServerMqHandle;

/**
 *  @brief      Task information for root server task.
 */
static Utils_Ptr            Rpe_rootServerTaskHandle;

/*===================== Static Function Declarations ========================*/

static void RpeServer_rootServerTask (
    uint32_t                    argc,
    Utils_Ptr                   argv);

static int32_t RpeServer_checkServerConfig (
    const Rpe_ServerConfig      *serverCfg,
    const Rpe_ServerClassConfig *serverClassCfg);
    
static int32_t RpeServer_checkAllServerConfigs ();

static void RpeServer_generateServerRecvMqName (
    char                        name[]);
    
static int32_t RpeServer_findServerClassConfig (
    uint32_t                    classId,
    const Rpe_ServerClassConfig **serverClassCfgRet);

static int32_t RpeServer_findServerConfig (
    char *                      name,
    const Rpe_ServerConfig    **serverCfgRet);
    
static void RpeServer_getClientConfig (
    Rpe_CallDesc                *desc);

/*===================== Function Definitions ================================*/

/******************************************************************************
 *  ========== RpeServer_init() ==========
 *
 *  See rpe_server.h
 *****************************************************************************/

int32_t RpeServer_init ()
{
    int32_t                     status = RPE_S_SUCCESS;
    uint16_t                    selfProcId;
    char                        rootServerMqName[RPE_MAX_MQ_NAME_LEN + 1];
    MessageQ_Params             mqParams;

    Utils_initSharedRegionAddressTable ();

    RPE_dspAlgHeapName   = "DSP_ALG_HEAP";
    RPE_dspAlgHeapHandle = NULL;

    RPE_dspAlgHeapHandle = (uint8_t *) memstp_getHeapHdlByName (RPE_dspAlgHeapName);
    
    if ((status = Utils_initMonitorTask ()) != RPE_S_SUCCESS)
        goto Error;
        
    if ((status = RpeServer_checkAllServerConfigs ()) != RPE_S_SUCCESS)
        goto Error;

    /* Open input MessageQ for the root server on this processor */
    selfProcId = MultiProc_self ();
    snprintf (rootServerMqName, RPE_MAX_MQ_NAME_LEN + 1, 
                                RPE_ROOT_SERVER_MQ_NAME_FORMAT, selfProcId);

    MessageQ_Params_init (&mqParams);
    if ((Rpe_rootServerMqHandle =
         MessageQ_create (rootServerMqName, &mqParams)) == NULL) {
        status = RPE_E_SERVER_IPC_CONN;
        goto Error;
    }

    /* Create the root server task */
    status = Utils_taskCreate (&Rpe_rootServerTaskHandle,
                               RpeServer_rootServerTask,
                               0, NULL,
                               Rpe_rootServerTaskConfig.stackSize,
                               Rpe_rootServerTaskConfig.osPriority,
                               Rpe_rootServerTaskConfig.taskName);

    if (RPE_S_SUCCESS != status) {
        status = RPE_E_OS_TASK_CREATE;
        goto Error;
    }
    
    goto Exit;

Error:
    if (NULL != Rpe_rootServerMqHandle)
        MessageQ_delete (&Rpe_rootServerMqHandle);

Exit:
    return (status);
}

/******************************************************************************
 *  ========== RpeServer_deInit() ==========
 *
 *  This is not yet implemented.
 *****************************************************************************/

int32_t RpeServer_deInit ()
{
    if (NULL != Rpe_rootServerMqHandle)
        MessageQ_delete (&Rpe_rootServerMqHandle);

    return (RPE_S_SUCCESS);
}

/** **************************************************************************
 *  @brief      Root server task that creates all RPE servers. It is started
 *              during system initialization of the remote core.
 *
 *  @param[in]  argc            Number of arguments
 *  @param[in]  argv            Array of task arguments
 *
 *  @retval     void            None
 *
 *  Created from Rpe_serverInit
 *****************************************************************************/

static void RpeServer_rootServerTask (
    uint32_t                    argc,
    Utils_Ptr                   argv)
{
    Rpe_CallDesc               *desc = NULL;
    MessageQ_QueueId            clientMsgqId;

    for ( ; ; ) {
        if (MessageQ_get (Rpe_rootServerMqHandle,
                          (MessageQ_Msg *) (&desc),
                          MessageQ_FOREVER) != MessageQ_S_SUCCESS) {
            break;
        }

        /* The root server task handles 'Create' and 'Get Config' commands */
        switch (desc->cmdType) {

            case RPE_CMD_CREATE:

                RpeServer_create (desc);
                break;
                
            case RPE_CMD_GETCONFIG:
                
                RpeServer_getClientConfig (desc);
                break;

            default:

                desc->status = RPE_E_INVALID_CMD;
                break;
        }

        clientMsgqId = MessageQ_getReplyQueue ((MessageQ_Msg) desc);
        if (MessageQ_put (clientMsgqId, (MessageQ_Msg) desc)
                                       != MessageQ_S_SUCCESS) {
            continue;
        }
        
        /* If the command is SHUTDOWN and clear messageQ and break out of the loop */
        if (RPE_CMD_SHUTDOWN == desc->cmdType)  {
            RpeServer_deInit ();
            break;
        }
     }
 
    Utils_taskExit (Rpe_rootServerTaskHandle);    /* Exit from the server task */


    return;
}

/******************************************************************************
 *  ========== RpeServer_defaultServerTask() ==========
 *
 *  See rpe_server.h
 *****************************************************************************/

void RpeServer_defaultServerTask (
    uint32_t                    argc,
    Utils_Ptr                   argv)
{
    Rpe_CallDesc               *desc = NULL;
    Rpe_ServerObj              *server;
    Utils_Ptr                   taskHandle;
    MessageQ_Handle             recvMqHandle;
    MessageQ_QueueId            clientMsgqId;
    uint8_t                     cmd;
    uint8_t                     serverDeleteDone;

    server = (Rpe_ServerObj *) argv;
    taskHandle = server->taskHandle;
    recvMqHandle = server->serverRecvMsgqHndl;

    for ( ; ; ) {
        if (MessageQ_get (recvMqHandle,
                          (MessageQ_Msg *) (&desc),
                          MessageQ_FOREVER) != MessageQ_S_SUCCESS) {
            break;
        }

        /* This task handles only Process, Control and Delete commands */
        cmd = desc->cmdType;
        switch (cmd) {

            case RPE_CMD_PROCESS:

                RpeServer_process (desc);
                break;

            case RPE_CMD_CONTROL:

                RpeServer_control (desc);
                break;

            case RPE_CMD_DELETE:
            
                RpeServer_delete (desc);
                if (desc->status == RPE_S_SUCCESS)
                    serverDeleteDone = TRUE;
                else
                    serverDeleteDone = FALSE;
                break;

            default:

                desc->status = RPE_E_INVALID_CMD;
                break;
        }

        clientMsgqId = MessageQ_getReplyQueue ((MessageQ_Msg) desc);
        if (MessageQ_put (clientMsgqId, (MessageQ_Msg) desc)
                                        != MessageQ_S_SUCCESS) {
            continue;
        }

        /* If the command is DELETE and deletion of server object has been
         * done successfully, break out of the loop */
        if ((RPE_CMD_DELETE == cmd) && (TRUE == serverDeleteDone)) {
            break;
        }
    }
    
    Utils_taskExit (taskHandle);    /* Exit from the server task */

    return;
}

/******************************************************************************
 *  ========== RpeServer_create() ==========
 *
 *  See rpe_server.h
 *****************************************************************************/

void RpeServer_create (
    Rpe_CallDesc               *desc)
{
    const Rpe_ServerConfig     *serverCfg;
    const Rpe_ServerClassConfig *serverClassCfg;
    char                       *name;
    Rpe_CreateCallDesc         *createCallDesc = (Rpe_CreateCallDesc *) desc;
    Rpe_ServerObj              *server = NULL;
    Rpe_ServerCreateFxn         createFxn;
    Rpe_Attributes             *instAttr;
    void                       *createParams;
    char                        recvMqName[RPE_MAX_MQ_NAME_LEN + 1];
    int32_t                     status;
    MessageQ_Params             mqParams;
    Error_Block                 eb;

    /* Find configuration record for the server */
    name = createCallDesc->name;
    
    /* Find configuration records for the server */
    if ((status = RpeServer_findServerConfig (name, &serverCfg)) 
                                                    != RPE_S_SUCCESS) {
        goto Error;
    }
    if ((status = RpeServer_findServerClassConfig (serverCfg->classId, 
                                         &serverClassCfg)) != RPE_S_SUCCESS) {
        goto Error;
    }

    /* Alocate server object */
    server = (Rpe_ServerObj *) Memory_alloc (NULL,
                                             serverClassCfg->serverHandleSize,
                                             RPE_MEM_ALLOC_ALIGNMENT, 
                                             &eb);
    if (NULL == server) {
        status = RPE_E_SERVER_NO_MEMORY;
        goto Error;
    }

    /* Initialize server object */
    server->serverConfig        = serverCfg;
    server->serverClassConfig   = serverClassCfg;
    server->isCreateCallDone    = FALSE;
    server->isDeleteDone        = FALSE;
    server->taskHandle          = NULL;
    server->serverRecvMsgqHndl  = NULL;

    /*
     * Now call the class specific create function using function pointer
     * in the server configuration structure. E.g. XdmServer_create
     */
    createFxn       = serverClassCfg->createFxn;
    instAttr        = &createCallDesc->instAttr;
    createParams    = (char *)createCallDesc
                      + RPE_ALIGNED_SIZE (sizeof (Rpe_CreateCallDesc));

    if ((status = createFxn (server, instAttr, createParams))
        != RPE_S_SUCCESS) {
        goto Error;
    }
    server->isCreateCallDone = TRUE;

    /* Create message queue to receive requests for the new server */
    RpeServer_generateServerRecvMqName (recvMqName);
    MessageQ_Params_init (&mqParams);
    if ((server->serverRecvMsgqHndl =
                        MessageQ_create (recvMqName, &mqParams)) == NULL) {
        status = RPE_E_SERVER_IPC_CONN;
        goto Error;
    }
    createCallDesc->serverMsgqId =
                            MessageQ_getQueueId (server->serverRecvMsgqHndl);

    status = Utils_taskCreate (&server->taskHandle,
                               serverClassCfg->serverTaskEntry,
                               1, server,
                               serverCfg->taskStackSize,
                               Utils_taskGetOsPriority (instAttr->priority),
                               serverCfg->name);

    if (RPE_S_SUCCESS != status) {
        status = RPE_E_OS_TASK_CREATE;
        goto Error;
    }
    
    /* Return server handle to the client */
    desc->serverHandle = server;

    goto Exit;

Error:

    /* If error, free up all resources */
    if (NULL != server) {

        MessageQ_Handle             serverRecvMsgqHndl;

        if (TRUE == server->isCreateCallDone)
            serverClassCfg->deleteFxn (server);

        if ((serverRecvMsgqHndl = server->serverRecvMsgqHndl) != NULL)
            MessageQ_delete (&serverRecvMsgqHndl);

        Memory_free (NULL, server, server->serverClassConfig->serverHandleSize);
    }
    desc->serverHandle = NULL;

Exit:
    desc->status = status;

    return;
}

/******************************************************************************
 *  ========== RpeServer_delete() ==========
 *
 *  See rpe_server.h
 *****************************************************************************/

void RpeServer_delete (
    Rpe_CallDesc               *desc)
{
    Rpe_ServerObj              *server = desc->serverHandle;
    const Rpe_ServerClassConfig *serverClassCfg;
    MessageQ_Handle             serverRecvMsgqHndl;

    server->isDeleteDone = FALSE;
    serverClassCfg = server->serverClassConfig;

    if ((desc->status = serverClassCfg->deleteFxn (server)) == RPE_S_SUCCESS) {
        if ((serverRecvMsgqHndl = server->serverRecvMsgqHndl) != NULL)
            MessageQ_delete (&serverRecvMsgqHndl);

        Memory_free (NULL, server, server->serverClassConfig->serverHandleSize);
    }

    return;
}

/******************************************************************************
 *  ========== RpeServer_control ==========
 *
 *  See rpe_server.h
 *****************************************************************************/

void RpeServer_control (
    Rpe_CallDesc               *desc)
{
    Rpe_ServerObj              *server;
    const Rpe_ServerClassConfig *serverClassCfg;
    int32_t                     status;
    uint8_t                     argCnt;
    FArg                        args[RPE_MAX_ARGCNT + 1];
    Rpe_FxnPtr                  controlFxn;
    Rpe_FxnPtr                  marshallControlFxn;
    
    server = desc->serverHandle;
    serverClassCfg = server->serverClassConfig;

    /* 
     * Calculate the argument pointers for the control call.
     * First parameter is the server handle. Rest of the parameters are passed
     * by the client.
     */
    args[0] = server;
    Rpe_getFunctionCallArgs (desc, &argCnt, &args[1]);

    /* 1. Call Control function */
    controlFxn = serverClassCfg->controlFxn;
    if ((status = Rpe_makeFunctionCall (controlFxn, argCnt + 1, args)
                                                      != RPE_S_SUCCESS)) {
        goto Exit;
    }

    /* 2. Call marshall function if present */
    if ((marshallControlFxn = serverClassCfg->marshallControlFxn) != NULL) {
        status = Rpe_makeFunctionCall (marshallControlFxn,
                                       argCnt + 1,
                                       args);
    }

Exit:
    desc->status = status;
    return;
}

/******************************************************************************
 *  ========== RpeServer_process ==========
 *
 *  See rpe_server.h
 *****************************************************************************/

void RpeServer_process (
    Rpe_CallDesc               *desc)
{
    Rpe_ServerObj              *server;
    const Rpe_ServerClassConfig *serverClassCfg;
    int32_t                     status;
    uint8_t                     argCnt;
    FArg                        args[RPE_MAX_ARGCNT + 1];
    Rpe_FxnPtr                  processFxn;
    Rpe_FxnPtr                  marshallProcessFxn;

    server = desc->serverHandle;
    serverClassCfg = server->serverClassConfig;

    /* 
     * Calculate the argument pointers for the control/process call.
     * First parameter is the server handle. Rest of the parameters are passed
     * by the client.
     */
    args[0] = server;
    Rpe_getFunctionCallArgs (desc, &argCnt, &args[1]);

    /* 1. Call Process function */
    processFxn = serverClassCfg->processFxn;
    if ((status = Rpe_makeFunctionCall (processFxn, argCnt + 1, args)
                                                      != RPE_S_SUCCESS)) {
        goto Exit;
    }

    /* 2. Call marshall function if present */
    if ((marshallProcessFxn = serverClassCfg->marshallProcessFxn) != NULL) {
        status = Rpe_makeFunctionCall (marshallProcessFxn,
                                       argCnt + 1,
                                       args);
    }

Exit:
    desc->status = status;
    return;
}

/** **************************************************************************
 *  @brief      Generate a messageq name to receive message in a server 
 *              instance from client.
 *
 *  @param[out] name            Generated messageq name.
 *
 *  @retval     RPE_S_SUCCESS   Success
 *
 *  Called from: RpeServer_create()
 *****************************************************************************/

static void RpeServer_generateServerRecvMqName (
    char                        name[])
{
    static uint32_t             curServerMqInstId = 0;
    uint32_t                    newMqInstId;
    uint16_t                    selfProcessorId;

    /* Get processor id for self */
    selfProcessorId = MultiProc_self ();

    newMqInstId = curServerMqInstId++;

    snprintf (name, RPE_MAX_MQ_NAME_LEN + 1, "RPE_SER_MQ_%u_%u", 
                                    (uint32_t)selfProcessorId, newMqInstId);

    return;
}

/** **************************************************************************
 *  @brief      Find a server class config structure from config db.
 *
 *  @param[in]  classId         Server class id.
 *  @param[out] serverClassCfgRet Server class config. structure.
 *
 *  @retval     RPE_S_SUCCESS   Success
 *
 *  Called from: RpeServer_create(), RpeServer_checkAllServerConfigs()
 *****************************************************************************/

static int32_t RpeServer_findServerClassConfig (
    uint32_t                    classId,
    const Rpe_ServerClassConfig **serverClassCfgRet)
{
    const Rpe_ServerClassConfig *serverClassCfg;
    int16_t                     i;
    int32_t                     status;
    
    *serverClassCfgRet = NULL;
    status = RPE_E_INVALID_SERVER_CLASS_ID;

    for (i = 0; ; i++) {
         
        serverClassCfg = Rpe_serverClassConfigArray[i];
        
        if (0 == serverClassCfg->classId)           /* Last entry */
            break;
         
        if (serverClassCfg->classId == classId) {
            *serverClassCfgRet = serverClassCfg;
            status = RPE_S_SUCCESS;
            break;
        }
    }
    
    return (status);
}

/** **************************************************************************
 *  @brief      Find a server config structure from config db.
 *
 *  @param[in]  name            Server name.
 *  @param[out] serverCfgRet    Pointer to server config. structure.
 *
 *  @retval     RPE_S_SUCCESS   Success
 *
 *  Called from: RpeServer_create()
 *****************************************************************************/

static int32_t RpeServer_findServerConfig (
    char                       *name,
    const Rpe_ServerConfig    **serverCfgRet)
{
    const Rpe_ServerConfig     *serverCfg;
    int16_t                     i;
    int32_t                     status;
    
    *serverCfgRet   = NULL;
    status          = RPE_E_SERVER_NOT_FOUND;

    for (i = 0; ; i++) {
    
        serverCfg = Rpe_serverConfigArray[i];
    
        if (NULL == serverCfg->name)                /* Last entry */
            break;
         
        if (0 == strcmp (name, serverCfg->name)) {
            *serverCfgRet = serverCfg;
            status = RPE_S_SUCCESS;
            break;
        }
    }
    
    return (status);
}

/** **************************************************************************
 *  @brief      Check the values in a server class configuration staructure.
 *
 *  @param[in]  serverClassCfg  Pointer to a server class configuration 
 *                              structure.
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  Called from: RpeServer_checkAllServerConfigs()
 *****************************************************************************/

static int32_t RpeServer_checkServerClassConfig (
    const Rpe_ServerClassConfig *serverClassCfg)
{
    int32_t         status = RPE_S_SUCCESS;

    if (0 == serverClassCfg->classId) {
        status = RPE_E_INVALID_SERVER_CLASS_ID;
    }
    else if (NULL == serverClassCfg->createFxn) {
        status = RPE_E_NULL_CREATE_FXN;
    }
    else if (NULL == serverClassCfg->deleteFxn) {
        status = RPE_E_NULL_DELETE_FXN;
    }
    else if (NULL == serverClassCfg->controlFxn) {
        status = RPE_E_NULL_CONTROL_FXN;
    }
    else if (NULL == serverClassCfg->processFxn) {
        status = RPE_E_NULL_PROCESS_FXN;
    }
    else if ((0 == serverClassCfg->numControlArgs) ||
             (RPE_MAX_ARGCNT < serverClassCfg->numControlArgs)) {
        status = RPE_E_INVALID_NUM_CONTROL_ARGS;
    }
    else if ((0 == serverClassCfg->numProcessArgs) ||
             (RPE_MAX_ARGCNT < serverClassCfg->numProcessArgs)) {
        status = RPE_E_INVALID_NUM_PROCESS_ARGS;
    }
    else if (NULL == serverClassCfg->serverTaskEntry) {
        status = RPE_E_NULL_SERVER_FXN;
    }
    else if (0 == serverClassCfg->serverHandleSize) {
        status = RPE_E_INVALID_SERVER_HANDLE_SIZE;
    }
    return (status);
}

/** **************************************************************************
 *  @brief      Check the values in a server configuration staructure.
 *
 *  @param[in]  serverCfg       Pointer to a server configuration structure.
 *  @param[in]  serverClassCfg  Pointer to a server class configuration 
 *                              structure.
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  Called from: RpeServer_checkAllServerConfigs()
 *****************************************************************************/

static int32_t RpeServer_checkServerConfig (
    const Rpe_ServerConfig      *serverCfg,
    const Rpe_ServerClassConfig *serverClassCfg)
{
    int32_t         status = RPE_S_SUCCESS;
    uint16_t        i;

    if (0 == serverCfg->classId) {
        status = RPE_E_INVALID_SERVER_CLASS_ID;
    }
    else if (0 == serverCfg->taskStackSize) {
        status = RPE_E_INVALID_TASK_STACK_SIZE;
    }
    else if (RPE_ALLBUFS_CPU_ACCESS_MODE_UNDEFINED == 
                                        serverCfg->inBufCpuAccessMode) {
        status = RPE_E_SERVER_BUF_ACCESS_UNDEFINED;
    }
    else if (RPE_ALLBUFS_CPU_ACCESS_MODE_UNDEFINED == 
                                        serverCfg->outBufCpuAccessMode) {
        status = RPE_E_SERVER_BUF_ACCESS_UNDEFINED;
    }
    else if (0 == serverCfg->sizeofCreateArgs) {
        status = RPE_E_INVALID_RPE_CREATE_PARAM_SIZE;
    }
    
    if (RPE_S_SUCCESS != status)
        goto Exit;
    
    /* Check size of control call args */
    for (i = 0; i < serverClassCfg->numControlArgs; i++) {
        if (0 == serverCfg->sizeofControlArgs[i]) {
            status = RPE_E_INVALID_CONTROL_ARG_SIZE;
            goto Exit;
        }
    }

    /* Check size of process call args */
    for (i = 0; i < serverClassCfg->numProcessArgs; i++) {
        if (0 == serverCfg->sizeofProcessArgs[i]) {
            status = RPE_E_INVALID_PROCESS_ARG_SIZE;
            goto Exit;
        }
    }

Exit:
    return (status);
}

/** **************************************************************************
 *  @brief      Check all server configuration staructures.
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  Called from: RpeServer_init()
 *****************************************************************************/

static int32_t RpeServer_checkAllServerConfigs ()
{
    const Rpe_ServerClassConfig *serverClassCfg;
    const Rpe_ServerConfig     *serverCfg;
    int16_t                     i;
    int32_t                     status = RPE_S_SUCCESS;

    /* First check all the server class config. structures */
    for (i = 0; ; i++) {
         
        serverClassCfg = Rpe_serverClassConfigArray[i];
        
        if (0 == serverClassCfg->classId)
            break;

        if ((status = RpeServer_checkServerClassConfig (serverClassCfg))
                                                        != RPE_S_SUCCESS) {
            goto Exit;
        }
    }
    
    /* Check all server config structures */
    for (i = 0; ; i++) {
    
        serverCfg = Rpe_serverConfigArray[i];
    
        if (NULL == serverCfg->name)
            break;
         
        if ((status = RpeServer_findServerClassConfig (serverCfg->classId,
                                          &serverClassCfg)) != RPE_S_SUCCESS) {
            goto Exit;
        }

        if ((status = RpeServer_checkServerConfig (serverCfg, serverClassCfg))
                                                        != RPE_S_SUCCESS) {
            goto Exit;
        }
    }
    
Exit:
    return (status);
}

/** **************************************************************************
 *  @brief      This function gets configuration information asked by the
 *              a client.
 *
 *  @param[in]  desc        Pointer to the call descriptor.
 *
 *  @retval     RPE_S_SUCCESS    Success
 */

static void RpeServer_getClientConfig (
    Rpe_CallDesc                *desc)
{
    Rpe_GetConfigCallDesc       *getCfgDesc = (Rpe_GetConfigCallDesc *)desc;
    uint16_t                    configIndex;
    const Rpe_ServerConfig     *serverCfg;
    const Rpe_ServerClassConfig *serverClassCfg;
    uint16_t                    i;
    int32_t                     status = RPE_S_SUCCESS;
    
    if ((configIndex = getCfgDesc->configIndex) >= Rpe_serverConfigCount) {
        status = RPE_E_INVALID_CONFIG_INDEX;
        goto Exit;
    }
    
    /* Return number of configurations available with server */
    getCfgDesc->configCount = Rpe_serverConfigCount;
    
    /* get server config structure and corresponding class config structure */
    serverCfg = Rpe_serverConfigArray[configIndex];
    if ((status = RpeServer_findServerClassConfig (serverCfg->classId,
                                          &serverClassCfg)) != RPE_S_SUCCESS)
        goto Exit;
    
    /* Copy config info from server config structures */
    strncpy (getCfgDesc->name, serverCfg->name, RPE_MAX_COMP_NAME_LEN);
    getCfgDesc->name[RPE_MAX_COMP_NAME_LEN] = '\0';
    
    getCfgDesc->classId             = serverCfg->classId;
    getCfgDesc->processorId         = MultiProc_self ();
    getCfgDesc->numControlArgs      = serverClassCfg->numControlArgs;
    getCfgDesc->numProcessArgs      = serverClassCfg->numProcessArgs;
    getCfgDesc->sizeofCreateArgs    = serverCfg->sizeofCreateArgs;
    
    for (i = 0; i < RPE_MAX_ARGCNT; i++) {
        getCfgDesc->sizeofControlArgs[i] = serverCfg->sizeofControlArgs[i];
        getCfgDesc->sizeofProcessArgs[i] = serverCfg->sizeofProcessArgs[i];
    }
    
Exit:   
    desc->status = status;
    return;
}

