
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
 *  @file       rpe_server.h
 *
 *  @brief      Defines data structures used by the RPE Server.
 * 
 */

#ifndef RPE_SERVER_H
#define RPE_SERVER_H

#include <ti/ipc/MessageQ.h>

#include "ti/rpe.h"
#include "ti/rpe_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 *  @brief      Typedef for struct Rpe_ServerObj.
 */
typedef struct Rpe_ServerObj Rpe_ServerObj;

/**
 *  @brief      Generic type definition for a server create function.
 *
 *  @param[in]  server          Pointer to an server object
 *  @param[in]  instAttr        Pointer to instance attribute structure passed
 *                              by client
 *  @param[in]  createParams    Algorithm create parameters passed by
 *                              client
 *  @retval     RPE_S_SUCCESS   Success
 */
typedef int32_t (*Rpe_ServerCreateFxn) (
    Rpe_ServerObj              *server,
    Rpe_Attributes             *instAttr,
    void                       *createParams);

/**
 *  @brief      Generic type definition for an server delete function.
 *
 *  @param[in]  server          Pointer to an server object
 *
 *  @retval     RPE_S_SUCCESS    Success
 */
typedef int32_t (*Rpe_ServerDeleteFxn) (
    Rpe_ServerObj              *server);

/**
 *  @brief      Generic type definition for RPE server task entry function.
 *
 *  @param[in]  argc            Number of arguments
 *  @param[in]  argv            Array of task arguments
 *
 *  @retval     void
 */
typedef void (*Rpe_ServerTaskEntry) (
    uint32_t                    argc,
    Utils_Ptr                   argv);

/**
 *  @brief      Defines structure for the server side configuration of a
 *              particular RPE server class.
 */
typedef struct Rpe_ServerClassConfig {

    /** RPE Implementation Class */
    uint32_t                    classId;

    /** Pointer to server create function */
    Rpe_ServerCreateFxn         createFxn;

    /** Pointer to server delete function */
    Rpe_ServerDeleteFxn         deleteFxn;

    /** Pointer to server control function */
    Rpe_FxnPtr                  controlFxn; 

    /** Pointer to server process function */    
    Rpe_FxnPtr                  processFxn; 

    /** Pointer to server process call marshall function */
    Rpe_FxnPtr                  marshallProcessFxn;

    /** Pointer to server control call marshall function */
    Rpe_FxnPtr                  marshallControlFxn;

    /** Number of arguments taken by a Control call. */
    uint8_t                     numControlArgs;

    /** Number of arguments taken by a Process call. */    
    uint8_t                     numProcessArgs;

    /** Pointer to server's task entry function */
    Rpe_ServerTaskEntry         serverTaskEntry; 

    /** Size of the server object structure */    
    uint16_t                    serverHandleSize;

} Rpe_ServerClassConfig;

/**
 *  @brief      Defines structure for the server side configuration of a
 *              particular RPE server.
 */
typedef struct Rpe_ServerConfig {

    /** Name of the RPE component. */
    char                       *name;

    /** RPE Implementation Class */
    uint32_t                    classId;

    /** Task stack size */
    uint32_t                    taskStackSize; 

    /** 
     *  CPU access mode for input buffers on  server side. 
     *
     *  @remarks It can hold access modes for max. 16 buffers
     */
    uint32_t                    inBufCpuAccessMode; 

    /** 
     *  CPU access mode for output buffers on server side. 
     *
     *  @remarks It can hold access modes for max. 16 buffers
     */
    uint32_t                    outBufCpuAccessMode;

    /** Size of the parameter structure for algorithm create call. */
    uint16_t                    sizeofCreateArgs;

    /** Sizes of the arguments to Control call. */
    uint16_t                    sizeofControlArgs[RPE_MAX_ARGCNT]; 

    /** Sizes of the arguments to Process call. */
    uint16_t                    sizeofProcessArgs[RPE_MAX_ARGCNT];

} Rpe_ServerConfig;

/**
 *  @brief      Defines structure for an RPE instance server object.
 */
struct Rpe_ServerObj {

    /** Pointer to the RPE component config. structure used by server */
    const Rpe_ServerConfig     *serverConfig;

    /** Pointer to the RPE component class config. structure used by server */
    const Rpe_ServerClassConfig *serverClassConfig;

    /** Flag - if the create function has been called */
    uint8_t                     isCreateCallDone;

    /** Flag - if the delete function has been called */
    uint8_t                     isDeleteDone;

    /** Handle to the task/thread used by server. */
    Utils_Ptr                   taskHandle;     

    /** Arguments passed to task by create function */    
    FArg                        taskArgs[1];

    /** MessageQ to receive msg in the server. */
    MessageQ_Handle             serverRecvMsgqHndl;
};

/**
 *  @brief      Array of server class configuration structures.
 */
extern const Rpe_ServerClassConfig *Rpe_serverClassConfigArray[];

/**
 *  @brief      Array of server implementation configuration structures.
 */
extern const Rpe_ServerConfig *Rpe_serverConfigArray[];

/**
 *  @brief      Number of algorithms integrated with the server.
 */
extern const uint32_t Rpe_serverConfigCount;

/**
 *  @brief      Default server task entry function.
 *
 *  @param[in]  argc            Number of arguments
 *  @param[in]  argv            Array of task arguments
 *
 *  @retval     void
 *
 *  Called from Rpe_createServer function using the 'serverTaskEntry' field
 *  specified in the server configuration structure.
 */
void  RpeServer_defaultServerTask (
    uint32_t                    argc,
    Utils_Ptr                   argv);
    
/**
 *  @brief      This function initializes the RPE server module in a remote
 *              processor firmware.
 *
 *  @retval     void    None
 *
 *  This function performs following:
 *  
 *  1) Creates root server task that receives all create requests.
 *  2) Opens a message queue to receive messages in the root server task.
 *  3) Creates a monitor task that deletes tasks and frees up task tasks when
 *     tasks exit.
 *
 *  It is called during firmware system initialization.
 */

int32_t RpeServer_init ();

/**
 *  @brief      This function is called by the root server to create an instance
 *              RPE server
 *
 *  @param[in]  desc        Pointer to create call descriptor.
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  This function performs following:
 *  
 *  1) Searches for RPE server configuration.
 *  2) Checks server config. calling RpeServer_checkServerConfig().
 *  3) Allocates a server object.
 *  2) Calls the server create function present in server config. structure.
 *  3) If separateServerTaskFlag (in config. structure) is true, 
 *     A) Creates a server task
 *     B) Creates a message queue to receive messages in the new task.
 *
 *  Called from Rpe_rootServerTask.
 */

void RpeServer_create (
    Rpe_CallDesc               *desc);

/**
 *  @brief      This function is called by the server task to delete a RPE
 *              server.
 *
 *  @param[in]  desc        Pointer to call descriptor.
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  This function performs following:
 *  
 *  1) Calls the server delete function present in server config. structure.
 *  2) If separateServerTaskFlag (in config. structure) is true, 
 *     deletes the task's message queue.
 *  3) Free up memory for server object.
 *
 *  Called from Rpe_rootServerTask, Rpe_defaultServerTask ...
 */

void RpeServer_delete (
    Rpe_CallDesc               *desc);
    
/**
 *  @brief      This function makes a call to the server's control 
 *              function.
 *
 *  @param[in]  desc        Pointer to the call descriptor.
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  This function performs following:
 *  
 *  1) Calls the Control function present in the server config.
 *  2) Calls the Marshall function if present in server config. structure.
 *
 *  Currently, it is assumed that no unmarshalling is required on server side
 *  before a control function is called.
 *
 *  Called from Rpe_rootServerTask, Rpe_defaultServerTask ...
 */

void RpeServer_control (
    Rpe_CallDesc                *desc);

/**
 *  @brief      This function makes a call to the server's process
 *              function.
 *
 *  @param[in]  desc        Pointer to the call descriptor.
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  This function performs following:
 *  
 *  1) Calls the Process function present in the server config.
 *  2) Calls the Marshall function if present in server config. structure.
 *
 *  Currently, it is assumed that no unmarshalling is required on server side
 *  before a process function is called.
 *
 *  Called from Rpe_rootServerTask, Rpe_defaultServerTask ...
 */

void RpeServer_process (
    Rpe_CallDesc                *desc);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* RPE_SERVER_H */
