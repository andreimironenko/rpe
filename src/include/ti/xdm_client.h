
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
 *  @file       xdm_client.h
 *
 *  @brief      Defines functions that implement client side marshalling and 
 *              unmarshalling functions for various types XDM algorithm calls.
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
#ifndef XDM_CLIENT_H
#define XDM_CLIENT_H

/* Basic types required by XDM interface */
#include <ti/Std.h>

/* XDM interface */
#include <ti/xdais/dm/xdm.h>

#include "ti/rpe_client.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 *  @brief      This function implements client-side marshalling of arguments 
 *              to XDM process call that has input/output buffer 
 *              descriptors of type XDM1_BufDesc. This function is called
 *              by the function Rpe_call on the client side before sending a 
 *              process call request to the server.
 *
 *  @param[in]  client          Client handle
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
 *
 *  This function performs following steps:
 *
 *  1) Make cache coherency operations on data buffers based on the access modes 
 *     passed by client in Rpe_Attributes parameter. It performs 
 *     cache-invalidate or cache-writeback if the access mode is read or write
 *     respectively. It calls Utils_performMemoryCacheOperation function to 
 *     perform cache operation.
 *
 *  2) Translates all input/ouput buffer addresses from host's local address
 *     to system address calling Utils_translateLocalAdrToSystemAdr.
 */

int32_t XdmClient_marshallXdm1BufDescArgs (
    Rpe_ClientObj              *client,
    XDM1_BufDesc               *inBufs,
    XDM1_BufDesc               *outBufs,
    FArg                        inArgs,
    FArg                        outArgs);

int32_t XdmEngine_marshallJpegBufDescArgsInClient (
    Rpe_ClientObj        *client,
    XDM1_BufDesc            *inBufs,
    XDM1_BufDesc            *outBufs,
    FArg                    inArgs,
    FArg                    outArgs);


/**
 *  @brief      This function implements client-side un-marshalling of arguments 
 *              to XDM process call that has input/output buffer 
 *              descriptors of type XDM1_BufDesc. This function is called
 *              by the function Rpe_call on the client side after the 
 *              process call is done by the server.
 *
 *  @param[in]  client          Client handle
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
 *
 *  This function performs following steps:
 *
 *  1) Translates all input/ouput buffer addresses from system address to
 *     host's local address. It calls Utils_translateSystemAdrToLocalAdr.
 */

int32_t XdmClient_unmarshallXdm1BufDescArgs (
    Rpe_ClientObj              *client,
    XDM1_BufDesc               *inBufs,
    XDM1_BufDesc               *outBufs,
    FArg                        inArgs,
    FArg                        outArgs);


int32_t XdmEngine_unmarshallJpegBufDescArgsInClient (
    Rpe_ClientObj        *client,
    XDM1_BufDesc            *inBufs,
    XDM1_BufDesc            *outBufs,
    FArg                    inArgs,
    FArg                    outArgs);

/**
 *  @brief      This function implements client-side marshalling of arguments 
 *              to XDM process call that has input/output buffer 
 *              descriptors of type XDM1_SingleBufDesc. This function is called
 *              by the function Rpe_call on the client side before sending a 
 *              process call request to the server.
 *
 *  @param[in]  client          Client handle
 *  @param[in,out] inBuf        Pointer to input buffer descriptor.
 *  @param[in,out] outBuf       Pointer to output buffer descriptor.
 *  @param[in]  inArgs          Pointer to input arguments.
 *  @param[out] outArgs         Pointer to ouput arguments.
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  It has been used with following types of XDM interfaces.
 *
 *  1) ISPHDEC1
 *  2) ISPHENC1
 *
 *  This function performs following steps:
 *
 *  1) Make cache coherency operations on data buffers based on the access modes 
 *     passed by client in Rpe_Attributes parameter. It performs 
 *     cache-invalidate or cache-writeback if the access mode is read or write
 *     respectively. It calls Utils_performMemoryCacheOperation function to 
 *     perform cache operation.
 *
 *  2) Translates all input/ouput buffer addresses from host's local address
 *     to system address calling Utils_translateLocalAdrToSystemAdr.
 */

int32_t XdmClient_marshallXdm1SingleBufDescArgs (
    Rpe_ClientObj              *client,
    XDM1_SingleBufDesc         *inBuf,
    XDM1_SingleBufDesc         *outBuf,
    FArg                        inArgs,
    FArg                        outArgs);

/**
 *  @brief      This function implements client-side un-marshalling of arguments 
 *              to XDM process call that has input/output buffer 
 *              descriptors of type XDM1_SingleBufDesc. This function is called
 *              by the function Rpe_call on the client side after the 
 *              process call is done by the server.
 *
 *  @param[in]  client          Client handle
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
 *
 *  This function performs following steps:
 *
 *  1) Translates all input/ouput buffer addresses from system address to
 *     host's local address. It calls Utils_translateSystemAdrToLocalAdr.
 */

int32_t XdmClient_unmarshallXdm1SingleBufDescArgs (
    Rpe_ClientObj              *client,
    XDM1_SingleBufDesc         *inBuf,
    XDM1_SingleBufDesc         *outBuf,
    FArg                        inArgs,
    FArg                        outArgs);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* XDM_CLIENT_H */
