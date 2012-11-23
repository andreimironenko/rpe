
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ti/Std.h>

#ifdef ___DSPBIOS___
#include <xdc/runtime/IHeap.h>
#endif

#ifdef ___LINUX___
#include <ti/syslink/utils/IHeap.h>
#endif

#include <ti/ipc/SharedRegion.h>
#include <ti/ipc/MultiProc.h>
#include <ti/syslink/ProcMgr.h>

#if defined (___DSPBIOS___)
#include <ti/sysbios/hal/Cache.h>
#else
#include <ti/syslink/utils/Cache.h>
#endif

#include <ldr_memseg.h>

#include "ti/system_utils.h"

#define RPE_MAX_SHARED_REGIONS       8

#define RPE_INVALID_REGION           -1

void                       *Utils_memCfgVirtualAddr = NULL;
static uint32_t Utils_shmemEntries = 0;

typedef struct Utils_SharedRegionEntry {
    uint8_t                     isValid;
    uint32_t                    localBase;
    uint32_t                    len;
    uint8_t                     cacheEnable;
    uint32_t                    systemBase;
} Utils_SharedRegionEntry;

Utils_SharedRegionEntry     Utils_sharedRegion[RPE_MAX_SHARED_REGIONS];

int32_t Utils_initSharedRegionAddressTable ()
{
    LDR_MemSeg                 *memSeg;
    uint16_t                    idx;
    int32_t                     srErrCode;
    SharedRegion_Entry          srEntry;
    uint8_t                     shRegionId;
    int32_t                     status;
    uint32_t                    proc_id;
    uint32_t                    flag;

    /* Initialize all entries in the table as invalid */
    for (idx = 0; idx < RPE_MAX_SHARED_REGIONS; idx++)
        Utils_sharedRegion[idx].isValid = FALSE;
#if ! defined (___DSPBIOS___)
    Utils_memCfgVirtualAddr =
        Utils_mapPhyAddr2UsrVirtual (LDR_CONFIG_ADDR_MEMCFG_BASE,
                                     LDR_MEMCFG_SPACE_SIZE);
#else
    Utils_memCfgVirtualAddr = (void *)LDR_CONFIG_ADDR_MEMCFG_BASE;
#endif

    if (Utils_memCfgVirtualAddr == NULL) {
        status = RPE_E_FAIL;
        goto Exit;
    }

    memSeg = (LDR_MemSeg *) ((uint8_t *)Utils_memCfgVirtualAddr
                             + sizeof (LDR_Memseg_Version_Hdr));

    /* 
     * 1) Locate each shared region in the memory segment table
     * 2) Locate corresponding IPC SharedRegion entry
     * 3) Populate the local table using info found in 1) and 2)
     */
    for (idx = 0; idx < LDR_MAX_MEMSEG; idx++, memSeg++) {
        if (memSeg->valid == FALSE)
            break;

        if (LDR_SEGMENT_TYPE_DYNAMIC_SHARED_HEAP == memSeg->seg_type) {
            shRegionId = memSeg->shared_region_id;

            /* Find the IPC SharedRegion entry */
            srErrCode = SharedRegion_getEntry (shRegionId, &srEntry);
            if (srErrCode != SharedRegion_S_SUCCESS) {
                printf ("    Error: could not find shared region entry\n");
                status = RPE_E_SHREG_NOTINIT;
                goto Exit;
            }

            Utils_sharedRegion[Utils_shmemEntries].systemBase = memSeg->system_addr;
            Utils_sharedRegion[Utils_shmemEntries].localBase = (uint32_t)srEntry.base;
            Utils_sharedRegion[Utils_shmemEntries].len = srEntry.len;
            Utils_sharedRegion[Utils_shmemEntries].cacheEnable = srEntry.cacheEnable;
            Utils_sharedRegion[Utils_shmemEntries++].isValid = TRUE;
        }
        else if (LDR_SEGMENT_TYPE_CMEM == memSeg->seg_type) {
            Utils_sharedRegion[Utils_shmemEntries].systemBase = memSeg->system_addr;
#if ! defined (___DSPBIOS___)
            Utils_sharedRegion[Utils_shmemEntries].localBase = (uint32_t) Utils_mapPhyAddr2UsrVirtual (memSeg->system_addr, memSeg->size);
#else
            Utils_sharedRegion[Utils_shmemEntries].localBase = memSeg->system_addr;
#endif
            Utils_sharedRegion[Utils_shmemEntries].len = memSeg->size;
            proc_id = MultiProc_self ();
            flag = (memSeg->cache_enable_mask) & ( 1 << proc_id );
            flag = (flag != 0) ? TRUE: FALSE;
            Utils_sharedRegion[Utils_shmemEntries].cacheEnable = flag;
            Utils_sharedRegion[Utils_shmemEntries++].isValid = TRUE;
        }
    }
    status = RPE_S_SUCCESS;

Exit:
    return (status);
}

int32_t Utils_translateLocalAdrToSystemAdr (
    Ptr                         localAdr,
    Ptr                        *systemAdr)
{
    Utils_SharedRegionEntry    *regionEntry;
    uint16_t                    i;
    int16_t                     regionId = RPE_INVALID_REGION;
    uint32_t                    offset;
    int32_t                     status;

    /* Try to find the entry that contains the address */
    for (i = 0, regionEntry = &Utils_sharedRegion[0];
         i < Utils_shmemEntries; i++, regionEntry++) {

        if ((regionEntry->isValid) &&
            ((uint32_t)localAdr >= regionEntry->localBase) &&
            ((uint32_t)localAdr <
             (regionEntry->localBase + regionEntry->len))) {
            regionId = i;
            break;
        }
    }

    if (RPE_INVALID_REGION == regionId) {
        status = RPE_E_SHREG_INVALID;
        goto Exit;
    }

    offset = (uint32_t)localAdr - regionEntry->localBase;

    *systemAdr = (Ptr) (regionEntry->systemBase + offset);

    status = RPE_S_SUCCESS;

Exit:
    return (status);
}

int32_t Utils_translateSystemAdrToLocalAdr (
    Ptr                         systemAdr,
    Ptr                        *localAdr)
{
    Utils_SharedRegionEntry    *regionEntry;
    uint16_t                    i;
    int16_t                     regionId = RPE_INVALID_REGION;
    uint32_t                    offset;
    int32_t                     status;

    /* Find the entry that contains the address */
    for (i = 0, regionEntry = &Utils_sharedRegion[0];
         i < Utils_shmemEntries; i++, regionEntry++) {

        if ((regionEntry->isValid) &&
            ((uint32_t)systemAdr >= regionEntry->systemBase) &&
            ((uint32_t)systemAdr <
             (regionEntry->systemBase + regionEntry->len))) {

            regionId = i;
            break;
        }
    }

    if (RPE_INVALID_REGION == regionId) {
        status = RPE_E_SHREG_INVALID;
        goto Exit;
    }

    offset = (uint32_t)systemAdr - regionEntry->systemBase;

    *localAdr = (Ptr) (regionEntry->localBase + offset);

    status = RPE_S_SUCCESS;

Exit:
    return (status);
}

int32_t Utils_isMemoryCached (
    Ptr                         localAdr,
    uint32_t                   *isCached)
{
    Utils_SharedRegionEntry    *regionEntry;
    uint16_t                    i;
    int32_t                     status = RPE_E_SHREG_INVALID;

    /* Try to find the entry that contains the address */
    for (i = 0, regionEntry = &Utils_sharedRegion[0];
         i < Utils_shmemEntries; i++, regionEntry++) {

        if ((regionEntry->isValid) &&
            ((uint32_t)localAdr >= regionEntry->localBase) &&
            ((uint32_t)localAdr <
             (regionEntry->localBase + regionEntry->len))) {

            *isCached = regionEntry->cacheEnable;
            status = RPE_S_SUCCESS;
            break;
        }
    }

    return (status);
}

int32_t Utils_performMemoryCacheOperation (
    Ptr                         addr,
    uint32_t                    nbytes,
    uint32_t                    cpuAccessMode)
{
    int32_t                     status = RPE_S_SUCCESS;
    uint32_t                    isMemCached;

    if (RPE_CPU_ACCESS_MODE_NONE == cpuAccessMode) {
        /* Nothing to do */
        goto Exit;
    }

    if ((status = Utils_isMemoryCached (addr, &isMemCached))
                                                != RPE_S_SUCCESS) {
        /* Could not find the address in any shared region */
        goto Exit;
    }

    if (TRUE == isMemCached) {
        if (RPE_CPU_ACCESS_MODE_READ == cpuAccessMode)
            Cache_inv (addr, nbytes, Cache_Type_ALL, TRUE);
        else
            Cache_wbInv (addr, nbytes, Cache_Type_ALL, TRUE);
    }

Exit:
    return (status);
}

void Utils_printSharedRegionInfo ()
{
    LDR_MemSeg                 *memSeg;
    uint16_t                    idx;
    int32_t                     srErrCode;
    SharedRegion_Entry          srEntry;
    uint8_t                     shRegionId;

    /* Initialize all entries in the table as invalid */
    for (idx = 0; idx < RPE_MAX_SHARED_REGIONS; idx++)
        Utils_sharedRegion[idx].isValid = FALSE;

    memSeg = (LDR_MemSeg *) (LDR_CONFIG_ADDR_MEMCFG_BASE +
                             sizeof (LDR_Memseg_Version_Hdr));

    for (idx = 0; idx < LDR_MAX_MEMSEG; idx++, memSeg++) {
        if (memSeg->valid == FALSE)
            break;

        if (LDR_SEGMENT_TYPE_DYNAMIC_SHARED_HEAP == memSeg->seg_type) {
            shRegionId = memSeg->shared_region_id;

            /* Find the IPC SharedRegion entry */
            srErrCode = SharedRegion_getEntry (shRegionId, &srEntry);
            if (srErrCode != SharedRegion_S_SUCCESS)
                printf ("    Error: could not find shared region entry\n");
        }
    }
    return;
}

#if ! defined (___DSPBIOS___)

/**
 * @name Utils_mapPhyAddr2UsrVirtual()
 * @brief Utils_mapPhyAddr2UsrVirtual function to map physical address to user virtual
 * @param phyAddr  : Physical address to be translated
 * @param len      : Length of memory block to be translated
 * @return none
 */
extern ProcMgr_Handle       procMgrHandle;

void *Utils_mapPhyAddr2UsrVirtual (
    uint32_t                    phyAddr,
    uint32_t                    len)
{
    int32_t                     status = 0;
    ProcMgr_AddrInfo            addrInfo;
    Ptr                         pUsrVirtAddr = NULL;
    ProcMgr_Handle              pSlaveProcHandle = NULL;

    /* Map the kernel space address to user space */
    addrInfo.addr[ProcMgr_AddrType_MasterPhys] = phyAddr;
    addrInfo.addr[ProcMgr_AddrType_SlaveVirt] = phyAddr;
    addrInfo.size = len;
    addrInfo.isCached = FALSE;

    pSlaveProcHandle = procMgrHandle;

    status = ProcMgr_translateAddr (pSlaveProcHandle,
                                    (Ptr) & pUsrVirtAddr,
                                    ProcMgr_AddrType_MasterUsrVirt,
                                    (Ptr) phyAddr, ProcMgr_AddrType_MasterPhys);
    if (status < 0) {
        status = ProcMgr_map (pSlaveProcHandle,
                              (ProcMgr_MASTERKNLVIRT |
                               ProcMgr_MASTERUSRVIRT),
                              &addrInfo, ProcMgr_AddrType_MasterPhys);
        if (status < 0) {
            pUsrVirtAddr = NULL;
        }
        status = ProcMgr_translateAddr (pSlaveProcHandle,
                                        (Ptr) & pUsrVirtAddr,
                                        ProcMgr_AddrType_MasterUsrVirt,
                                        (Ptr) phyAddr,
                                        ProcMgr_AddrType_MasterPhys);

        if (status < 0) {
            pUsrVirtAddr = NULL;
        }
    }

    return (pUsrVirtAddr);
}

/**
 *  @brief                  Utils_unmapPhyAddr unmaps previous mapped  physical 
 *                          address
 *
 *  @param[in]  phyAddr     Physical address to be unmapped
 *  @param[in]  len         Length of memory block to be unmapped
 *
 *  @return     none
 */
Void Utils_unmapPhyAddr (
    UInt32                      phyAddr,
    UInt32                      len)
{

    Int32                       status = 0;
    ProcMgr_AddrInfo            addrInfo;
    ProcMgr_Handle              pSlaveProcHandle = NULL;
    Ptr                         pUsrVirtAddr = NULL;
    Ptr                         pKnlVirtAddr = NULL;

    addrInfo.addr[ProcMgr_AddrType_MasterPhys] = phyAddr;
    addrInfo.addr[ProcMgr_AddrType_SlaveVirt] = phyAddr;
    addrInfo.size = len;
    addrInfo.isCached = FALSE;

    pSlaveProcHandle = procMgrHandle;
    status = ProcMgr_translateAddr (pSlaveProcHandle,
                                    (Ptr) & pUsrVirtAddr,
                                    ProcMgr_AddrType_MasterUsrVirt,
                                    (Ptr) phyAddr, ProcMgr_AddrType_MasterPhys);
    if (status < 0) {
        printf ("Error in ProcMgr_translateAddr [0x%x]\n", (uint) status);

    }
    else {
        printf ("ProcMgr_translateAddr Status [0x%x]"
                " User Virtual Address [0x%x]\n", (uint) status,
                (uint) pUsrVirtAddr);
    }

    addrInfo.addr[ProcMgr_AddrType_MasterUsrVirt] = (UInt32) pUsrVirtAddr;

    status = ProcMgr_translateAddr (pSlaveProcHandle,
                                    (Ptr) & pKnlVirtAddr,
                                    ProcMgr_AddrType_MasterKnlVirt,
                                    (Ptr) phyAddr, ProcMgr_AddrType_MasterPhys);
    if (status < 0) {
        printf ("Error in ProcMgr_translateAddr [0x%x]\n", (uint32_t)status);

    }
    else {
        printf ("ProcMgr_translateAddr Status [0x%x]"
                " KNl Virtual Address [0x%x]\n", (uint) status,
                (uint) pKnlVirtAddr);
    }

    addrInfo.addr[ProcMgr_AddrType_MasterKnlVirt] = (UInt32) pKnlVirtAddr;

    status = ProcMgr_unmap (pSlaveProcHandle,
                            (ProcMgr_MASTERKNLVIRT |
                             ProcMgr_MASTERUSRVIRT),
                            &addrInfo, ProcMgr_AddrType_MasterPhys);
    if (status < 0) {
        printf ("ProcMgr_unmap Failed status: 0x%x", (uint) status);
    }
    else {
        printf ("ProcMgr_unmap Success ");
    }
    printf ("Unmap done:%x:%d:%d\n", (uint) phyAddr, (int)len, (int)status);

    return;
}
#endif
