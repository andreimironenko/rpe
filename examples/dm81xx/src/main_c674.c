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
 *  @file  main_c674.c
 *  @brief This file contains the main () of the DSP application.
 *
 *  @rev 1.0
 *******************************************************************************
 */

/*******************************************************************************
*                             Compilation Control Switches
*******************************************************************************/
/* None */

/*******************************************************************************
*                             INCLUDE FILES
*******************************************************************************/

/* -------------------- system and platform files ----------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <xdc/std.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/IHeap.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Timestamp.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Task.h>
#include <xdc/cfg/global.h>
#include <ti/ipc/Ipc.h>
#include <ti/sysbios/utils/Load.h>
#include "platform.h"
#include <sys_top.h>

/*-------------------------program files --------------------------------------*/
/* None */

/*******************************************************************************
 * EXTERNAL REFERENCES NOTE : only use if not found in header file
*******************************************************************************/

/*-----------------------data declarations -----------------------------------*/
/* None */

/*--------------------- function prototypes ----------------------------------*/
Void Dsp_AppTask (UArg arg1, UArg arg2);

/*******************************************************************************
 * PUBLIC DECLARATIONS Defined here, used elsewhere
 ******************************************************************************/
/* None */

/*******************************************************************************
 * PRIVATE DECLARATIONS Defined here, used only here
 ******************************************************************************/
/* None */

/*--------------------------- macros  ----------------------------------------*/
/** 
 *  DSP_APP_TASK_SLEEP_DURATION, the Test Application sleeps for this 
 *  duration before verifying the indication for application termination.
*/
#define  DSP_APP_TASK_SLEEP_DURATION (1000u)
#ifdef PLATFORM_TI811X
#define  FIRMWARE_STOP_ADDR        (0x9FFFFFFC)
#else
#define  FIRMWARE_STOP_ADDR        (0xBFFFEFFC)
#endif
/* volatile variables */
    volatile int rootserverStopFlag;

/** 
********************************************************************************
 *  @fn     sysTop_Task
 *  @brief  This function does the platform specific initialization and just 
 *          sleeps.
 * 
 *  @param[in ]  arg1  : Not used, Reserved for future use
 *  @param[in ]  arg2  : Not used, Reserved for future use
 * 
 *  @returns none 
********************************************************************************
*/
Void sysTop_Task (UArg arg1, UArg arg2)
{
    sysTop_process(NULL);
}


/** 
********************************************************************************
 *  @fn     Dsp_AppTask
 *  @brief  This function does the platform specific initialization and just 
 *          sleeps.
 * 
 *  @param[in ]  arg1  : Not used, Reserved for future use
 *  @param[in ]  arg2  : Not used, Reserved for future use
 * 
 *  @returns none 
********************************************************************************
*/

Void Dsp_AppTask (UArg arg1, UArg arg2)
{
    int *rootServerFlagLoc = (int *) FIRMWARE_STOP_ADDR;
    
    rootserverStopFlag = (*rootServerFlagLoc);
    
    /*------------------------------------------------------------------------*/
    /* Initiailize the system. The initialization can include driver, syslink */
    /*------------------------------------------------------------------------*/
    platform_init ();

    while (!rootserverStopFlag) {
        rootserverStopFlag = (*rootServerFlagLoc);
        Task_sleep (DSP_APP_TASK_SLEEP_DURATION);
    }
    
    platform_deinit ();
}

/** 
********************************************************************************
 *  @fn     main
 *  @brief  This function is entry to the DSP side of test application. 
 *
 *  @param[in]   : None
 *
 *  @return 0
********************************************************************************
*/

volatile unsigned int debug = 0;

int main (void)
{
    while (debug) {
        if (debug) {
            debug = 1;
        }
    }
    Ipc_start ();

    BIOS_start ();
    return (0);

}

