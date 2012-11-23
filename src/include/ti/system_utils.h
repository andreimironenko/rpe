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

#ifndef SYSTEM_UTILS_H
#define SYSTEM_UTILS_H

#include <stdint.h>

#include <ti/Std.h>
#include "ti/rpe.h"
#include "ti/rpe_types.h"

int32_t Utils_initSharedRegionAddressTable (void);

int32_t Utils_translateLocalAdrToSystemAdr (Ptr localAdr, Ptr *systemAdr);

int32_t Utils_translateSystemAdrToLocalAdr (Ptr systemAdr, Ptr *localAdr);

int32_t Utils_isMemoryCached (Ptr localAdr, uint32_t *isCached);

int32_t Utils_performMemoryCacheOperation(Ptr addr, uint32_t nbytes, uint32_t cpuAccessMode);

int32_t Utils_initMonitorTask();

void Utils_deinitMonitorTask();

int32_t Utils_taskGetOsPriority(Rpe_ProcessingPriority priority);

int32_t Utils_taskCreate(Utils_Ptr *pTask, void *pFunc, uint32_t uArgc,
                         void *pArgv, uint32_t uStackSize, int32_t priority,
                         char *pName);

void Utils_taskExit(Utils_Ptr pTask);

void* Utils_mapPhyAddr2UsrVirtual (uint32_t phyAddr, uint32_t len);

#endif /* SYSTEM_UTILS_H */
