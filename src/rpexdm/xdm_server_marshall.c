
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

#include <xdc/runtime/IHeap.h>
#include <ti/ipc/SharedRegion.h>
#include <ti/ipc/MessageQ.h>

#include <ti/Std.h>

/* XDM interface */
#include <ti/xdais/dm/xdm.h>

/* Get structure definitions for IAUDDEC1 interface */
#include <ti/xdais/dm/iauddec1.h>

/* Get structure definitions for IVIDDEC3 interface */
#include <ti/xdais/dm/ividdec3.h>

#include "ti/xdm_server.h"
#include "ti/system_utils.h"

/*
 * XDM defines following types of buffer descriptor structures. We need to
 * write marshall/unmarshall functions for each of these types.
 * 1) XDM_BufDesc
 * 2) XDM_SingleBufDesc
 * 3) XDM1_SingleBufDesc
 * 4) XDM2_SingleBufDesc
 * 5) XDM1_BufDesc
 * 6) XDM2_BufDesc
 */

/*===================== Function Definitions ================================*/

/******************************************************************************
 *  ========== XdmServer_marshallXdm1BufDescArgs() ==========
 *
 *  Marshall functions for Xdm1BufDesc. See xdm_server.h
 *****************************************************************************/

int32_t XdmServer_marshallXdm1BufDescArgs (
    Rpe_ServerObj              *server,
    XDM1_BufDesc               *inBufs,
    XDM1_BufDesc               *outBufs,
    FArg                        inArgs,
    FArg                        outArgs)
{
    int32_t                     status = RPE_S_SUCCESS;
    const Rpe_ServerConfig     *serverConfig = server->serverConfig;
    uint16_t                    numBufs;
    uint16_t                    i;
    XDM1_SingleBufDesc         *singleBufDesc;
    uint32_t                    cpuAccessMode;

    /* 
     * For all input and output buffers
     *    a) Perform cache operation if it was accessed by CPU
     */ 

    /* Handle buffer pointers present in inBufs structure */
    for (i = 0, numBufs = 0;
         ((numBufs < inBufs->numBufs) && (i < XDM_MAX_IO_BUFFERS)); i++) {

        singleBufDesc = &inBufs->descs[i];
        if (NULL != singleBufDesc->buf) {

            /* Find CPU Access Mode */
            cpuAccessMode =
                RPE_GET_CPU_ACCESS_MODE (serverConfig->inBufCpuAccessMode, i);

            /* Perform cache operation based on CPU Access Mode */
            if (RPE_CPU_ACCESS_MODE_NONE != cpuAccessMode) {
                if ((status = Utils_performMemoryCacheOperation (
                                        singleBufDesc->buf,
                                        singleBufDesc->bufSize,
                                        cpuAccessMode)) != RPE_S_SUCCESS) {
                    goto Exit;
                }
            }
        }
        numBufs++;
    }

    /* Handle buffer pointers present in outBufs structure */
    for (i = 0, numBufs = 0;
         ((numBufs < outBufs->numBufs) && (i < XDM_MAX_IO_BUFFERS)); i++) {

        singleBufDesc = &outBufs->descs[i];
        if (NULL != singleBufDesc->buf) {

            /* Find CPU Access Mode */
            cpuAccessMode =
                RPE_GET_CPU_ACCESS_MODE (serverConfig->outBufCpuAccessMode, i);

            /* Perform cache operation based on CPU Access Mode */
            if (RPE_CPU_ACCESS_MODE_NONE != cpuAccessMode) {
                if ((status = Utils_performMemoryCacheOperation (
                                        singleBufDesc->buf,
                                        singleBufDesc->bufSize,
                                        cpuAccessMode)) != RPE_S_SUCCESS) {
                    goto Exit;
                }
            }
        }
        ++numBufs;
    }

Exit:
    return (status);
}

/******************************************************************************
 *  ========== XdmServer_marshallXdm1SingleBufDescArgs() ==========
 *
 *  Marshall functions for XDM1_SingleBufDesc. See xdm_server.h
 *****************************************************************************/

int32_t XdmServer_marshallXdm1SingleBufDescArgs (
    Rpe_ServerObj              *server,
    XDM1_SingleBufDesc         *inBuf,
    XDM1_SingleBufDesc         *outBuf,
    FArg                        inArgs,
    FArg                        outArgs)
{
    int32_t                     status = RPE_S_SUCCESS;
    const Rpe_ServerConfig     *serverConfig = server->serverConfig;
    uint32_t                    cpuAccessMode;

    /* 
     * For all input and output buffers
     *    a) Perform cache operation if it was accessed by CPU
     */ 
     
    /* Handle buffer pointer present in inBuf structure */
    if (NULL != inBuf->buf) {

        /* Find CPU Access Mode */
        cpuAccessMode =
            RPE_GET_CPU_ACCESS_MODE (serverConfig->inBufCpuAccessMode, 0);

        /* Perform cache operation based on CPU Access Mode */
        if (RPE_CPU_ACCESS_MODE_NONE != cpuAccessMode) {
            if ((status = Utils_performMemoryCacheOperation (
                                            inBuf->buf,
                                            inBuf->bufSize,
                                            cpuAccessMode)) != RPE_S_SUCCESS) {
                goto Exit;
            }
        }
    }

    /* Handle buffer pointer present in outBuf structure */
    if (NULL != outBuf->buf) {

        /* Find CPU Access Mode */
        cpuAccessMode =
            RPE_GET_CPU_ACCESS_MODE (serverConfig->outBufCpuAccessMode, 0);

        /* Perform cache operation based on CPU Access Mode */
        if (RPE_CPU_ACCESS_MODE_NONE != cpuAccessMode) {
            if ((status = Utils_performMemoryCacheOperation (
                                            outBuf->buf,
                                            outBuf->bufSize,
                                            cpuAccessMode)) != RPE_S_SUCCESS) {
                goto Exit;
            }
        }
    }

Exit:
    return (status);
}
