
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

/* Include files for xdais interfaces  */
#include <string.h>
#include <stdio.h>

#include <ti/Std.h>

/* XDAIS Interface */
#include <ti/xdais/ialg.h>

/* SysBios Heap Allocation Interfaces */
#include <xdc/runtime/IHeap.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/Error.h>
#include <xdc/cfg/global.h>

#include "ti/xdm_server.h"
#include "ti/system_utils.h"

/*===================== Local Type Definitions ========================*/
extern uint8_t *RPE_dspAlgHeapHandle;
extern uint8_t *globaL2heap;

/**
 *  @brief  Generic function pointer type for XDM Algorithm Control function
 *
 *  @param[in]  alg         XDAIS Alg Handle
 *  @param[in]  cmdId       XDM Control Command Id
 *  @param[in]  dynParams   XDM Dynamic Params
 *  @param[out] xdmStatus   XDM Return Status
 */
typedef int32_t (*XdmServer_ControlFxn) (
    IALG_Handle                 alg,
    uint32_t                    cmdId,
    FArg                        dynParams,
    FArg                        xdmStatus);

/**
 *  @brief  Generic function pointer type for XDM Algorithm Process function 
 *
 *  @param[in]  alg         XDAIS Alg Handle
 *  @param[in]  cmdId       XDM Control Command Id
 *  @param[in]  dynParams   XDM Dynamic Params
 *  @param[out] xdmStatus   XDM Return Status
 */
typedef int32_t (*XdmServer_ProcessFxn) (
    IALG_Handle                 alg,
    FArg                        inBufs,
    FArg                        outBufs,
    FArg                        inArgs,
    FArg                        outArgs);
 
/*===================== Static Function Declarations ========================*/

static int32_t _XdmServer_allocXdaisAlgMemory (
    IALG_MemRec                 memTab[],
    Int                         numObjs);

static void _XdmServer_freeXdaisAlgMemory (
    IALG_MemRec                 memTab[],
    Int                         numRecs);

/*===================== Function Definitions ================================*/

/******************************************************************************
 *  ========== XdmServer_create() ==========
 *
 *  See xdm_server.h
 *****************************************************************************/

int32_t XdmServer_create (
    Rpe_ServerObj              *server,
    Rpe_Attributes             *instAttr,
    void                       *createParams)
{
    XdmServer_ServerObj        *xdmServer = (XdmServer_ServerObj *) server;
    XDM_Fxns                   *xdmFxns;
    IALG_Fxns                  *ialgFxns;
    IALG_MemRec                *memTab = NULL;
    Int                         numRecs;
    uint16_t                    memTabSize;
    IALG_Handle                 alg;
    IALG_Fxns                  *fxnsPtr;
    XdmServer_ServerConfig     *xdmServerCfg;
    uint8_t                     algAllocDone = FALSE;
    int32_t                     status, i;
    uint8_t                    *ptr;
    Error_Block                 eb;

#ifdef DEBUG
    fprintf (stderr, "XdmServer_createServer: Enter\n");
#endif
    xdmServer->alg = NULL;

    xdmServerCfg = (XdmServer_ServerConfig *) (server->serverConfig);

    /* Check that all fields in the config. structure are initialized */
    if ((xdmFxns = xdmServerCfg->xdmFxns) == NULL) {
        status = RPE_E_XDM_NULL_XDMFXNS;
        goto Error;
    }

    ialgFxns = &(xdmFxns->ialgFxns);

    if (NULL == ialgFxns->algAlloc) {
        status = RPE_E_XDM_NULL_ALGALLOC_FXN;
        goto Error;
    }
    if (NULL == ialgFxns->algFree) {
        status = RPE_E_XDM_NULL_ALGFREE_FXN;
        goto Error;
    }
    if (NULL == ialgFxns->algInit) {
        status = RPE_E_XDM_NULL_ALGINIT_FXN;
        goto Error;
    }
    if (NULL == xdmFxns->algControlFxn) {
        status = RPE_E_XDM_NULL_CONTROL;
        goto Error;
    }
    if (NULL == xdmFxns->algProcessFxn) {
        status = RPE_E_XDM_NULL_PROCESS;
        goto Error;
    }
    
    /* Get number of XDAIS alg memtab entries */
    xdmServer->numRecs = numRecs = (ialgFxns->algNumAlloc != NULL)
        ? ialgFxns->algNumAlloc () : IALG_DEFMEMRECS;

    /* Allocate memtab entries */
    memTabSize = numRecs * sizeof(IALG_MemRec);      
    
    if ((memTab = (IALG_MemRec *) Memory_alloc ((IHeap_Handle) RPE_dspAlgHeapHandle,
                                                memTabSize,
                                                RPE_MEM_ALLOC_ALIGNMENT,
                                                &eb)) == NULL) {
        status = RPE_E_SERVER_NO_MEMORY;
        goto Error;
    }
    
    /* Clear the memTab memory */
    ptr = (uint8_t *) memTab;
    for (i = 0; i < memTabSize; i++)
    {
      ptr[i] = 0;
    }
    
    /* Save memTab in the server record */
    xdmServer->memTab = memTab;

    /* Fill up memtab entries */
    if ((numRecs = ialgFxns->algAlloc (createParams, &fxnsPtr, memTab)) <= 0) {
        status = RPE_E_XDM_INVALID_CREAT_PARAMS;
        goto Error;
    }

    /* Allocated algorithm memory using memtab entries */
    if ((status = _XdmServer_allocXdaisAlgMemory (memTab, numRecs))
                                                            != RPE_S_SUCCESS) {
        goto Error;
    }
    algAllocDone = TRUE;

    /* Initialize alg handle */
    alg = (IALG_Handle) memTab[0].base;
    alg->fxns = ialgFxns;

    /* 
     * Call alg initialize function with the memory it requested.
     * If algInit successful return the alg object's handle 
     */
    if (ialgFxns->algInit (alg, memTab, NULL, createParams) != IALG_EOK) {
        status = RPE_E_XDM_ALGINIT;
        goto Error;
    }

    /* Put algHandle in the server handle */
    xdmServer->alg = alg;

    status = RPE_S_SUCCESS;
    goto Exit;

Error:
    /* Free all memory resources */

    /* Call algFree to free all instance memory, saved memTab recs. */
    if (TRUE == algAllocDone) {
        ialgFxns->algFree (alg, memTab);
        _XdmServer_freeXdaisAlgMemory (memTab, numRecs);
    }

    if (NULL != memTab)
        Memory_free ((IHeap_Handle) RPE_dspAlgHeapHandle, memTab, memTabSize);

Exit:
    return (status);
}

/******************************************************************************
 *  ========== XdmServer_delete () ==========
 *
 *  See xdm_server.h
 *****************************************************************************/

int32_t XdmServer_delete (
    Rpe_ServerObj              *server)
{
    XdmServer_ServerObj        *xdmServer = (XdmServer_ServerObj *) server;
    IALG_Handle                 alg;
    IALG_MemRec                *memTab;
    Int                         numRecs;
    uint16_t                    memTabSize;
    IALG_Fxns                  *ialgFxns;
    int32_t                     status;

    if ((alg = xdmServer->alg) == NULL) {           /* Should not happen */
        status = RPE_E_XDM_NULL_ALG;
        goto Exit;
    }

    if (NULL == alg->fxns) {                        /* Should not happen */
        status = RPE_E_XDM_NULL_ALGFXNS;
        goto Exit;
    }

    ialgFxns = alg->fxns;
    numRecs = (NULL != ialgFxns->algNumAlloc) ? ialgFxns->algNumAlloc ()
                                              : IALG_DEFMEMRECS;

    /* Get the memTab memory that was allocated at create time */
    memTab = xdmServer->memTab;

    memTab[0].base = alg;
    numRecs = ialgFxns->algFree (alg, memTab);

    /* Free up algorithm memory */
    _XdmServer_freeXdaisAlgMemory((IALG_MemRec *) memTab, numRecs);

    /* Get the memTab memory size that was allocated */
    memTabSize = xdmServer->numRecs * sizeof (IALG_MemRec);
    /* Free memTab memory */
    Memory_free((IHeap_Handle) RPE_dspAlgHeapHandle, memTab, memTabSize);
    
    status = RPE_S_SUCCESS;

Exit:
    return (status);
}

void _XdmServer_freeXdaisAlgMemory(IALG_MemRec memTab[], Int numRecs)
{
    Int idx;
    uint8_t                 *handle;
    uint8_t                 *RPE_dspIramHeapHandle = globaL2heap;
    
    for (idx = 0; idx < numRecs; idx++) 
    {
        if (memTab[idx].base != NULL) 
        {
            if ( memTab[idx].space > IALG_SARAM2 ) 
            { 
                handle = RPE_dspAlgHeapHandle;
            }
            else
            {
                handle = RPE_dspIramHeapHandle;
            }

            /* Freeing a buffer of size 0 is not possible, so we allocate     */
            /* 8 bytes during creation. This is taken care during deletion    */
            if (memTab[idx].size == 0)
            {
                memTab[idx].size = 8;
            }
            
            Memory_free ((IHeap_Handle) handle, memTab[idx].base, memTab[idx].size);
        }
    }
    return;
}

int32_t _XdmServer_allocXdaisAlgMemory(IALG_MemRec memTab[], Int numObjs)
{
    Int                     idx;
    Error_Block             eb;
    uint8_t                 *handle;
    uint8_t                 *RPE_dspIramHeapHandle = globaL2heap;

    for (idx = 0; idx < numObjs; idx++) 
    {
        if ( memTab[idx].space > IALG_SARAM2 ) 
        { 
            handle = RPE_dspAlgHeapHandle;
        }
        else
        {
            handle = RPE_dspIramHeapHandle;
        }

        /* Allocation a buffer of size 0 is not possible, so we allocate      */
        /* 8 bytes. This is taken care during deletion also                   */
        if (memTab[idx].size == 0)
        {
            memTab[idx].size = 8;
        }
        
        memTab[idx].base = Memory_alloc((IHeap_Handle) handle,
                                         memTab[idx].size,
                                         memTab[idx].alignment,
                                         &eb);

        if (NULL == memTab[idx].base) {
        
            _XdmServer_freeXdaisAlgMemory(memTab, idx);
            
            return (RPE_E_XDAIS_NO_MEMORY);
        }
        
        memset(memTab[idx].base, 0, memTab[idx].size);
    }

    return (RPE_S_SUCCESS);
}

/******************************************************************************
 *  ========== XdmServer_process () ==========
 *
 *  See xdm_server.h
 *****************************************************************************/

int32_t XdmServer_process (
    Rpe_ServerObj              *server,
    FArg                        inBufs,
    FArg                        outBufs,
    FArg                        inArgs,
    FArg                        outArgs)
{
    XdmServer_ServerConfig     *xdmServerCfg;
    XdmServer_ServerObj        *xdmServer;
    XDM_Fxns                   *xdmFxns;
    IALG_Fxns                  *ialgFxns;
    IALG_Handle                 alg;
    int32_t                     status;
    Rpe_FxnPtr                  algProcessFxn;

#ifdef DEBUG
    fprintf (stderr, "XdmServer_processServer: Enter\n");
#endif

    /* 
     * Retrieve ialgFxns and pointer to XDM process function from 
     * the server config record 
     */
    xdmServerCfg = (XdmServer_ServerConfig *) (server->serverConfig);
    xdmFxns = xdmServerCfg->xdmFxns;
    ialgFxns = &xdmFxns->ialgFxns;
    algProcessFxn = xdmFxns->algProcessFxn;

    /* Get alg handle from the server handle */
    xdmServer = (XdmServer_ServerObj *) server;
    alg = xdmServer->alg;

    /* 
     * It should have been already ensured that 
     * 1) ialgFxns is NOT NULL
     * 2) alg is NOT NULL
     */

    /* Call algActivate if NOT NULL */
    if (NULL != ialgFxns->algActivate)
        ialgFxns->algActivate (alg);

    /* Call XDM process */
    status = ((XdmServer_ProcessFxn) (algProcessFxn)) (alg, inBufs, outBufs, 
                                                       inArgs, outArgs);
    
    /* Call algDeactivate if NOT NULL */
    if (NULL != ialgFxns->algDeactivate)
        ialgFxns->algDeactivate (alg);

    return (status);
}

/******************************************************************************
 *  ========== XdmServer_control () ==========
 *
 *  See xdm_server.h
 *****************************************************************************/

int32_t XdmServer_control (
    Rpe_ServerObj              *server,
    FArg                        arg1,
    FArg                        arg2,
    FArg                        arg3)
{
    XdmServer_ServerConfig     *xdmServerCfg;
    XdmServer_ServerObj        *xdmServer;
    IALG_Handle                 alg;
    int32_t                     status;
    XDM_Fxns                   *xdmFxns;
    Rpe_FxnPtr                  algControlFxn;

    /* Retrieve pointer to XDM control function from the server config record */
    xdmServerCfg = (XdmServer_ServerConfig *) (server->serverConfig);
    xdmFxns = xdmServerCfg->xdmFxns;
    algControlFxn = xdmFxns->algControlFxn;

    /* 
     * Get alg handle from the server handle. The create call should
     * have ensured that alg handle is NOT NULL.
     */
    xdmServer = (XdmServer_ServerObj *) server;
    alg = xdmServer->alg;

    /* Call XDM control */
    status = ((XdmServer_ControlFxn) (algControlFxn)) (alg,
                                                       *(uint32_t *)arg1, 
                                                       arg2,
                                                       arg3);
    return (status);
}
