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
 *******************************************************************************
 *  @file  platform_c674.c
 *  @brief This file contains all Functions related to platform initialization
 *         and deinitialization
 *
 *  @rev 1.0
 *******************************************************************************
 */

/*******************************************************************************
*                             Compilation Control Switches
*******************************************************************************/
/* None */

/* -------------------- system and platform files ----------------------------*/
#include <xdc/std.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/Diags.h>
#include <ti/sysbios/family/c64p/Cache.h>
#include <ti/ipc/Ipc.h>

#include "ti/rpe.h"
#include "ti/system_utils.h"
#include "ldr_memseg.h"
#include "mem_setup.h"
#include "system_init.h"
#include "platform.h"

extern RpeServer_init();

/** 
 ******************************************************************************
 *  @brief  Calls various initialization routines for different modules.
 *          
 *          Following modules are initialized :-
 *          SysLink, ldrmemcfg, RPE
 *
 *  @return none
 ******************************************************************************
 */
void platform_init ()
{
    MemCfg_Error memcfgErrCode = MemCfg_ErrorNone;

    /*-----------------------------------------------------------------------*/
    /* Initialize the SysLink                                                */
    /*-----------------------------------------------------------------------*/

    System_procInit ();

    /*-----------------------------------------------------------------------*/
    /* Create the global pool of SRs and Heaps required by application       */
    /*-----------------------------------------------------------------------*/

    memcfgErrCode = ldrmemcfg_createSRsHeaps(NULL, NULL);
    if (MemCfg_ErrorNone != memcfgErrCode) {
        printf ("Dynamic Memory Configuration failed\n");
    }

    /*-----------------------------------------------------------------------*/
    /* Initialize RPE Framework                                              */
    /*-----------------------------------------------------------------------*/
    
    RpeServer_init();
}

/** 
 ******************************************************************************
 *  @brief  Calls various de-initialization routines for different modules.
 *          
 *          Following modules are initialized :-
 *          SysLink, ldrmemcfg, RPE
 *
 *  @return none
 ******************************************************************************
 */
void platform_deinit ()
{
    /*-----------------------------------------------------------------------*/
    /* De-initialize the SysLink                                             */
    /*-----------------------------------------------------------------------*/

    System_procDeInit ();

    Ipc_stop ();

}
