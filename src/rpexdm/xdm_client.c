
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
#include <stdlib.h>
#include <ti/sdo/linuxutils/cmem/include/cmem.h>
#include "ti/xdm_client.h"
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

static Ptr marshallCmemPtr(Ptr localPtr, int32_t size, uint32_t cpuAccessMode)
{
    /* Manage the cache */
    if (RPE_CPU_ACCESS_MODE_NONE != cpuAccessMode) {
        if (cpuAccessMode == RPE_CPU_ACCESS_MODE_READ) {
            if (CMEM_cacheInv(localPtr, size))
                return 0;
        }
        else {
            if (CMEM_cacheWbInv(localPtr, size))
                return 0;
        }
    }

    /* Translate the user space pointer */
    return (Ptr) CMEM_getPhys(localPtr);
}

/******************************************************************************
 *  ========== XdmEngine_marshallJpegBufDescArgsInClient() ==========
 *
 *  Marshall functions for Xdm1BufDesc. See xdm_client.h
 *****************************************************************************/
int32_t XdmEngine_marshallJpegBufDescArgsInClient (
    Rpe_ClientObj        *client,
    XDM1_BufDesc            *inBufs,
    XDM1_BufDesc            *outBufs,
    FArg                    inArgs,
    FArg                    outArgs)
{
    uint16_t                numBufs;
    uint16_t                i;
    Ptr                     localPtr;
    Ptr                     systemPtr;
    XDM1_SingleBufDesc      *singleBufDesc;
    uint32_t                cpuAccessMode;
    
#if  DEBUG_ENGINE_API 
    fprintf (stderr, "Entering Function: %s\n", __FUNCTION__);
#endif
    
    /* 
     * 1) for all input buffers
     *    a) cache-writeback-invalidate
     *    b) translate local address to system address
     *
     * 2) for all output buffers
     *    a) cache-invalidate
     *    b) translate local address to system address
     */
    
    /* Translate local addresses present in the inBufs to system addresses */
    for (i = 0, numBufs = 0;
         ((numBufs < inBufs->numBufs) && (i < XDM_MAX_IO_BUFFERS)); i++) {
         
        singleBufDesc = &inBufs->descs[i];
        if ((localPtr = singleBufDesc->buf) != NULL) {
            /* Find CPU Access Mode */
            cpuAccessMode =
              RPE_GET_CPU_ACCESS_MODE (client->instAttr.inBufCpuAccessMode, i);

            systemPtr = marshallCmemPtr(localPtr, singleBufDesc->bufSize,
                                        cpuAccessMode);
            if (!systemPtr)
                return RPE_E_FAIL;
            inBufs->descs[i].buf = systemPtr;

            /* Clear .accessMask; the local processor won't access this buf */
            inBufs->descs[i].accessMask = 0;

            numBufs++;
        }
    }
    
    /* Translate local addresses present in the outBufs to system addresses */
    for (i = 0, numBufs = 0; 
         ((numBufs < outBufs->numBufs) && (i < XDM_MAX_IO_BUFFERS)); i++) {
         
        singleBufDesc = &outBufs->descs[i];
        if ((localPtr = singleBufDesc->buf) != NULL) {
            /* Find CPU Access Mode */
            cpuAccessMode =
             RPE_GET_CPU_ACCESS_MODE (client->instAttr.outBufCpuAccessMode, i);

            systemPtr = marshallCmemPtr(localPtr, singleBufDesc->bufSize,
                                        cpuAccessMode);
            if (!systemPtr)
                return RPE_E_FAIL;
            outBufs->descs[i].buf = systemPtr;

            ++numBufs ;
        }
    }
#if  DEBUG_ENGINE_API 
    fprintf (stderr, "Exiting Function: %s\n", __FUNCTION__);
#endif

    return RPE_S_SUCCESS;
}

/******************************************************************************
 *  ========== XdmEngine_unmarshallJpegBufDescArgsInClient() ==========
 *
 *  Marshall functions for Xdm1BufDesc. See xdm_client.h
 *****************************************************************************/
int32_t XdmEngine_unmarshallJpegBufDescArgsInClient (
    Rpe_ClientObj        *client,
    XDM1_BufDesc            *inBufs,
    XDM1_BufDesc            *outBufs,
    FArg                    inArgs,
    FArg                    outArgs)
{
#if  DEBUG_ENGINE_API 
    fprintf (stderr, "Entering Function: %s\n", __FUNCTION__);
#endif
    
    return (RPE_S_SUCCESS);
}

/******************************************************************************
 *  ========== XdmClient_marshallXdm1BufDescArgs() ==========
 *
 *  Marshall functions for Xdm1BufDesc. See xdm_client.h
 *****************************************************************************/

int32_t XdmClient_marshallXdm1BufDescArgs (
    Rpe_ClientObj              *client,
    XDM1_BufDesc               *inBufs,
    XDM1_BufDesc               *outBufs,
    FArg                        inArgs,
    FArg                        outArgs)
{
    int32_t                     status = RPE_S_SUCCESS;
    uint16_t                    numBufs;
    uint16_t                    i;
    Ptr                         localPtr;
    Ptr                         systemPtr;
    XDM1_SingleBufDesc         *singleBufDesc;
    uint32_t                    cpuAccessMode;

    /* 
     * For all input and output buffers
     *    a) Perform cache operation if it was accessed by CPU
     *    b) Translate local address to system address
     */

    /* Handle buffer pointers present in inBufs structure */
    for (i = 0, numBufs = 0;
         ((numBufs < inBufs->numBufs) && (i < XDM_MAX_IO_BUFFERS)); i++) {

        singleBufDesc = &inBufs->descs[i];
        if ((localPtr = singleBufDesc->buf) != NULL) {

            /* Find CPU Access Mode */
            cpuAccessMode =
              RPE_GET_CPU_ACCESS_MODE (client->instAttr.inBufCpuAccessMode, i);

            /* Perform cache operation based on CPU Access Mode */
            if (RPE_CPU_ACCESS_MODE_NONE != cpuAccessMode) {
                if ((status = Utils_performMemoryCacheOperation (localPtr,
                                       singleBufDesc->bufSize, cpuAccessMode))
                                                            != RPE_S_SUCCESS) {
                    goto Exit;
                }
            }
            /* 
             * Translate local addresses present in the inBufs to 
             * system addresses 
             */
            if ((status = Utils_translateLocalAdrToSystemAdr (localPtr,
                                               &systemPtr)) != RPE_S_SUCCESS) {
                goto Exit;
            }
            inBufs->descs[i].buf = systemPtr;

            /* Clear .accessMask; the local processor won't access this buf */
            inBufs->descs[i].accessMask = 0;

            numBufs++;
        }
    }

    /* Handle buffer pointers present in outBufs structure */
    for (i = 0, numBufs = 0;
         ((numBufs < outBufs->numBufs) && (i < XDM_MAX_IO_BUFFERS)); i++) {

        singleBufDesc = &outBufs->descs[i];
        if ((localPtr = singleBufDesc->buf) != NULL) {

            /* Find CPU Access Mode */
            cpuAccessMode =
             RPE_GET_CPU_ACCESS_MODE (client->instAttr.outBufCpuAccessMode, i);

            /* Perform cache operation based on CPU Access Mode */
            if (RPE_CPU_ACCESS_MODE_NONE != cpuAccessMode) {
                if ((status = Utils_performMemoryCacheOperation (localPtr,
                                        singleBufDesc->bufSize, cpuAccessMode))
                                                            != RPE_S_SUCCESS) {
                    goto Exit;
                }
            }

            /* 
             * Translate local addresses present in the outBufs to 
             * system addresses 
             */
            if ((status = Utils_translateLocalAdrToSystemAdr (localPtr,
                                             &systemPtr)) != RPE_S_SUCCESS) {
                goto Exit;
            }

            outBufs->descs[i].buf = systemPtr;

            ++numBufs;
        }
    }
Exit:
    return (status);
}

/******************************************************************************
 *  ========== XdmClient_unmarshallXdm1BufDescArgs() ==========
 *
 *  Unmarshall functions for Xdm1BufDesc. See xdm_client.h
 *****************************************************************************/

int32_t XdmClient_unmarshallXdm1BufDescArgs (
    Rpe_ClientObj              *client,
    XDM1_BufDesc               *inBufs,
    XDM1_BufDesc               *outBufs,
    FArg                        inArgs,
    FArg                        outArgs)
{
    int32_t                     status = RPE_S_SUCCESS;
    uint16_t                    numBufs;
    uint16_t                    i;
    Ptr                         localPtr;
    Ptr                         systemPtr;

    /* 
     * For all input and output buffers
     *    a) Translate system address to local address 
     */

    /* Translate system addresses present in the inBufs to local addresses */
    for (i = 0, numBufs = 0;
         ((numBufs < inBufs->numBufs) && (i < XDM_MAX_IO_BUFFERS)); i++) {

        if ((systemPtr = inBufs->descs[i].buf) != NULL) {

            if ((status = Utils_translateSystemAdrToLocalAdr (systemPtr,
                                                  &localPtr)) != RPE_S_SUCCESS) {
                goto Exit;
            }

            inBufs->descs[i].buf = localPtr;

            numBufs++;
        }
    }

    /* Translate system addresses present in the outBufs to local addresses */
    for (i = 0, numBufs = 0;
         ((numBufs < outBufs->numBufs) && (i < XDM_MAX_IO_BUFFERS)); i++) {

        if ((systemPtr = outBufs->descs[i].buf) != NULL) {

            if ((status = Utils_translateSystemAdrToLocalAdr (systemPtr,
                                                  &localPtr)) != RPE_S_SUCCESS) {
                goto Exit;
            }

            outBufs->descs[i].buf = localPtr;

            ++numBufs;
        }
    }

Exit:
    return (status);
}

/******************************************************************************
 *  ========== XdmClient_marshallXdm1SingleBufDescArgs() ==========
 *
 *  Marshall functions for XDM1_SingleBufDesc. See xdm_client.h
 *****************************************************************************/

int32_t XdmClient_marshallXdm1SingleBufDescArgs (
    Rpe_ClientObj              *client,
    XDM1_SingleBufDesc         *inBuf,
    XDM1_SingleBufDesc         *outBuf,
    FArg                        inArgs,
    FArg                        outArgs)
{
    int32_t                     status = RPE_S_SUCCESS;
    Ptr                         localPtr;
    Ptr                         systemPtr;
    uint32_t                    cpuAccessMode;

    /* 
     * For all input and output buffers
     *    a) Perform cache operation if it was accessed by CPU
     *    b) Translate local address to system address
     */

    /* Handle buffer pointer present in inBuf structure */
    if ((localPtr = inBuf->buf) != NULL) {
    
        /* Find CPU Access Mode */
        cpuAccessMode =
            RPE_GET_CPU_ACCESS_MODE (client->instAttr.inBufCpuAccessMode, 0);
            
        /* Perform cache operation based on CPU Access Mode */
        if (RPE_CPU_ACCESS_MODE_NONE != cpuAccessMode) {
            if ((status = Utils_performMemoryCacheOperation (localPtr,
                                             inBuf->bufSize, cpuAccessMode))
                                                            != RPE_S_SUCCESS) {
                goto Exit;
            }
        }

        /* Translate local address present in the inBuf to system addresses */
        if ((status = Utils_translateLocalAdrToSystemAdr (localPtr,
                                             &systemPtr)) != RPE_S_SUCCESS) {
            goto Exit;
        }
        inBuf->buf = systemPtr;
    }

    /* Handle buffer pointer present in outBuf structure */
    if ((localPtr = outBuf->buf) != NULL) {
    
        /* Find CPU Access Mode */
        cpuAccessMode =
            RPE_GET_CPU_ACCESS_MODE (client->instAttr.outBufCpuAccessMode, 0);
            
        /* Perform cache operation based on CPU Access Mode */
        if (RPE_CPU_ACCESS_MODE_NONE != cpuAccessMode) {
            if ((status = Utils_performMemoryCacheOperation (localPtr,
                                           outBuf->bufSize, cpuAccessMode))
                                                            != RPE_S_SUCCESS) {
                goto Exit;
            }
        }

        /* Translate local address present in the inBuf to system address */
        if ((status = Utils_translateLocalAdrToSystemAdr (localPtr,
                                             &systemPtr)) != RPE_S_SUCCESS) {
            goto Exit;
        }
        outBuf->buf = systemPtr;
    }

Exit:
    return (status);
}

/******************************************************************************
 *  ========== XdmClient_unmarshallXdm1SingleBufDescArgs() ==========
 *
 *  Unmarshall functions for XDM1_SingleBufDesc. See xdm_client.h
 *****************************************************************************/

int32_t XdmClient_unmarshallXdm1SingleBufDescArgs (
    Rpe_ClientObj              *client,
    XDM1_SingleBufDesc         *inBuf,
    XDM1_SingleBufDesc         *outBuf,
    FArg                        inArgs,
    FArg                        outArgs)
{
    int32_t                     status = RPE_S_SUCCESS;
    Ptr                         localPtr;
    Ptr                         systemPtr;

    /* 
     * For all input and output buffers
     *    a) Translate system address to local address 
     */

    /* Translate system address present in the inBuf to local address */
    if ((systemPtr = inBuf->buf) != NULL) {
        if ((status = Utils_translateSystemAdrToLocalAdr (systemPtr,
                                              &localPtr)) != RPE_S_SUCCESS) {
            goto Exit;
        }
        inBuf->buf = localPtr;
    }

    /* Translate system address present in the outBuf to local address */
    if ((systemPtr = outBuf->buf) != NULL) {
        if ((status = Utils_translateSystemAdrToLocalAdr (systemPtr,
                                              &localPtr)) != RPE_S_SUCCESS) {
            goto Exit;
        }
        outBuf->buf = localPtr;
    }

Exit:
    return (status);
}
