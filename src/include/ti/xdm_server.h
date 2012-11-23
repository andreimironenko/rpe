
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
 *  @file       xdm_server.h
 *
 *  @brief      Defines data structures and various functions that implement
 *              generic XDM server. 
 *
 *              Currently, it has functions to integrate following types of XDM
 *              codecs.
 *
 *              1) Audio Decode
 *              2) Audio Encode
 *              3) Speech Decode
 *              4) Speech Encode
 * 
 */

#ifndef XDM_RPE_H
#define XDM_RPE_H

/* Basic types required by XDM interface */
#include <xdc/std.h>

/* XDM interface */
#include <ti/xdais/dm/xdm.h>

#include "ti/rpe_server.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 *  @brief      Defines generic function pointer table for TI XDM interfaces.
 *              All XDM interfaces like IAUDDEC1, IAUDENC1, ISPHDEC1, ISPHENC1
 *              etc. has function pointer tables IAUDDEC1_Fxns, IAUDENC1_Fxns,
 *              ISPHDEC1_Fxns, ISPHENC1_Fxns etc. that have three common 
 *              entries:
 *                  1) ialg, 2) process, 3) control
 *
 *              However, there is no single type definition to generalize it.
 *              So, here we define a generic function pointer table for all
 *              XDM interfaces.
 */
 
 typedef struct XDM_Fxns {
 
    /** XDAIS algorithm interface. */
    IALG_Fxns                   ialgFxns;
    
    /** Pointer to XDM algorithm process function */
    Rpe_FxnPtr                  algProcessFxn;    
    
    /** Pointer to XDM algorithm control function */
    Rpe_FxnPtr                  algControlFxn;

} XDM_Fxns;    

/**
 *  @brief      Defines structure for the server side configuration of 
 *              XDM server. It extends the base server configuration structure
 *              RPE_ServerConfig.
 *
 *              @sa     Rpe_ServerConfig
 */
typedef struct XdmServer_ServerConfig {

    /** Base server configuration structure */
    Rpe_ServerConfig            serverConfig;

    /** If the algorithm uses shared internal memory as scratch.
      *
      *   @remarks Currently, this feature is not implemented.
      */
    uint8_t                     useSharedInternalScratchMemoryFlag; 

    /** If the algorithm uses shared shared DMA channel
     *
     *  @remarks Currently, this feature is not implemented.
     */             
    uint8_t                     useSharedDmaChannelFlag;
    
    /** If the algorithm uses IRES resource manager.
     *
     *  @remarks Currently, this feature is not implemented.
     */     
    uint8_t                     useIresFlag;    
    
    /** Pointer to XDM function table */
    XDM_Fxns                   *xdmFxns;

} XdmServer_ServerConfig;

/**
 *  @brief      Defines structure for an XDM server object. It is used by 
 *              server. It extends the base server object structure
 *              Rpe_ServerObj.
 *
 *              @sa     Rpe_ServerObj
 */
typedef struct XdmServer_ServerObj {

    /** Base server object structure */
    Rpe_ServerObj               server;   

    /** XDAIS algorithm object handle */ 
    IALG_Handle                 alg;    

    /** XDAIS algorithm memory records */
    IALG_MemRec                *memTab;   

    /** Number of memory records */
    uint16_t                    numRecs;    
} XdmServer_ServerObj;

/**
 *  @brief      This function creates server for an XDM algorithm.
 *
 *  @param[in]  server          Pointer to XDM server object
 *  @param[in]  instAttr        Pointer to component attribute structure passed by
 *                              client
 *  @param[in]  createParams    XDM algorithm create parameters passed by
 *                              client
 *  @retval     RPE_S_SUCCESS    Success
 */
int32_t XdmServer_create (
    Rpe_ServerObj              *server,
    Rpe_Attributes             *instAttr,
    void                       *createParams);

/**
 *  @brief      This function deletes an XDM algorithm server.
 *
 *  @param[in]  server          Pointer to an server object
 *
 *  @retval     RPE_S_SUCCESS    Success
 */
int32_t XdmServer_delete (
    Rpe_ServerObj              *server);

/**
 *  @brief      This function implements XDM server process call.
 *
 *  @param[in]  server          Pointer to XDM server object
 *  @param[in,out] inBufs       Pointer to input buffer descriptors.
 *  @param[in,out] outBufs      Pointer to output buffer descriptors.
 *  @param[in]  inArgs          Pointer to input arguments.
 *  @param[out] outArgs         Pointer to ouput arguments.
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  This function performs following steps:
 *
 *  1) Calls algActivate function from ialgFxns function pointer table.
 *  2) Calls XDM algorithm process function.
 *  3) Calls algDeactivate function from ialgFxns function pointer table.
 */
int32_t XdmServer_process (
    Rpe_ServerObj              *server,
    FArg                        inBufs,
    FArg                        outBufs,
    FArg                        inArgs,
    FArg                        outArgs);

/**
 *  @brief      This function implements XDM server control call.
 *
 *  @param[in]  server          Pointer to XDM server object
 *  @param[in]  id              Pointer to command id
 *  @param[in]  params          Pointer to dynamic parameters
 *  @param[out] status          Pointer to ouput results.
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  This function performs following steps:
 *
 *  1) Calls XDM algorithm control function.
 */
int32_t XdmServer_control (
    Rpe_ServerObj              *server,
    FArg                        id,
    FArg                        params,
    FArg                        status);

/**
 *  @brief      This function implements server-side marshalling of arguments 
 *              to XDM server process call that has input/output buffer 
 *              descriptors of type XDM1_BufDesc. This function is called by
 *              the server after the server process call is over.
 *
 *  @param[in]  server          Pointer to XDM server object
 *  @param[in,out] inBufs       Pointer to input buffer descriptors.
 *  @param[in,out] outBufs      Pointer to output buffer descriptors.
 *  @param[in]  inArgs          Pointer to input arguments.
 *  @param[out] outArgs         Pointer to ouput arguments.
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  It has been used with following types of XDM interfaces.
 *
 *  1) IAUDDEC1
 *  2) IAUDENC1
 *  3) IIMGDEC1
 */
int32_t XdmServer_marshallXdm1BufDescArgs (
    Rpe_ServerObj              *server,
    XDM1_BufDesc               *inBufs,
    XDM1_BufDesc               *outBufs,
    FArg                        inArgs,
    FArg                        outArgs);

/**
 *  @brief      This function implements server-side marshalling of arguments 
 *              to XDM server process call that has input/output buffer 
 *              descriptors of type XDM1_SingleBufDesc. This function is called
 *              by the server after the server process call is over.
 *
 *  @param[in]  server          Pointer to XDM server object
 *  @param[in,out] inBuf        Pointer to input buffer descriptors.
 *  @param[in,out] outBuf       Pointer to output buffer descriptors.
 *  @param[in]  inArgs          Pointer to input arguments.
 *  @param[out] outArgs         Pointer to ouput arguments.
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  It has been used with following types of XDM interfaces.
 *
 *  1) ISPHDEC1
 *  2) ISPHENC1
 */
int32_t XdmServer_marshallXdm1SingleBufDescArgs (
    Rpe_ServerObj              *server,
    XDM1_SingleBufDesc         *inBuf,
    XDM1_SingleBufDesc         *outBuf,
    FArg                        inArgs,
    FArg                        outArgs);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* XDM_RPE_H */
