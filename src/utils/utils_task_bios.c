
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

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/hal/Hwi.h>

#include <ti/sysbios/heaps/HeapMem.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/Error.h>
#include <ti/sysbios/knl/Clock.h>

#include "ti/rpe.h"
#include "ti/rpe_types.h"
#include "ti/rpe_server_task_config.h"
#include "ti/system_utils.h"

#define TASK_NAME_SIZE  20
#define TASK_MIN_WAIT_TIME 8
#define TASK_MAX_WAIT_TIME (TASK_MIN_WAIT_TIME << 4)

#define Utils_getTicks(mSec) ((mSec) * 1000 / Clock_tickPeriod)

typedef struct Utils_Task {
    struct Utils_Task          *next;

    /** Handle to the task context structure */
    Task_Handle                 task;

    /** Name of the task */
    char                        name[TASK_NAME_SIZE + 1];

    /** Pointer to the task stack memory */
    void                       *stackPtr;

    /** Size of the task stack */
    uint32_t                    stackSize;

    /** task priority */
    int32_t                     priority;

    /** flag to check if task got created */
    uint32_t                    isCreated;
} Utils_Task;

typedef struct Utils_ExitingTaskQue {
    Utils_Task                 *head;
    Utils_Task                 *tail;
} Utils_ExitingTaskQue;

static int32_t Utils_taskDelete (Utils_Ptr pTask);

static Utils_ExitingTaskQue Utils_exitingTaskQue = { NULL, NULL };

static Utils_Ptr            Rpe_monitorTaskHandle;

static Semaphore_Handle     Rpe_monitorTaskSem;

static uint32_t             Rpe_monitortaskExitFlag;

static void  Rpe_monitorTaskEntry (
    uint32_t                    argc,
    Utils_Ptr                   argv);

int32_t Utils_taskGetOsPriority (
    Rpe_ProcessingPriority priority)
{
    int32_t                     osPriority;

    switch (priority) {

        case RPE_PROCESSING_PRIORITY_LOW:
            osPriority = Rpe_OsTaskPriorityLow;
            break;

        case RPE_PROCESSING_PRIORITY_HIGH:
            osPriority = Rpe_OsTaskPriorityHigh;
            break;

        case RPE_PROCESSING_PRIORITY_MEDIUM:
        default:
            osPriority = Rpe_OsTaskPriorityMedium;
            break;
    }

    return osPriority;
}

int32_t Utils_taskCreate (
    Utils_Ptr                  *pTask,
    void                       *pFunc,
    uint32_t                    uArgc,
    void                       *pArgv,
    uint32_t                    uStackSize,
    int32_t                     priority,
    char                       *pName)
{
    int32_t                     retStatus = RPE_S_SUCCESS;
    Utils_Task                 *pHandle = NULL;
    Task_Params                 taskParams;
    Error_Block                 eb;

    *pTask = NULL;
    /*Note: -1 is a valid priority in Sys BIOS */

    if ((pFunc == NULL) || (priority > 31) || (priority < -1)) {
        retStatus = RPE_E_INVALIDARG;
        goto EXIT;
    }

    pHandle = (Utils_Task *) Memory_alloc (NULL,
                                           sizeof (Utils_Task),
                                           RPE_MEM_ALLOC_ALIGNMENT, 
                                           &eb);
    if (NULL == pHandle) {
        retStatus = RPE_E_UTILS_NO_MEMORY;
        goto EXIT;
    }

    pHandle->isCreated = FALSE;
    pHandle->task = NULL;
    pHandle->next = NULL;

    strncpy (pHandle->name, pName, TASK_NAME_SIZE);
    pHandle->name[TASK_NAME_SIZE] = '\000';
    pHandle->stackSize = uStackSize;
    pHandle->priority = priority;

    /* Allocate memory for task stack */
    pHandle->stackPtr = Memory_alloc (NULL,
                                      pHandle->stackSize,
                                      RPE_MEM_ALLOC_ALIGNMENT, 
                                      &eb);
    if (NULL == pHandle->stackPtr) {
        retStatus = RPE_E_UTILS_NO_MEMORY;
        goto EXIT;
    }

    Task_Params_init (&taskParams);

    taskParams.arg0 = uArgc;
    taskParams.arg1 = (uint32_t)pArgv;
    taskParams.priority = priority;
    taskParams.stack = pHandle->stackPtr;
    taskParams.stackSize = uStackSize;
    taskParams.instance->name = (xdc_String) pHandle->name;

    /* Create the task */
    pHandle->task = Task_create ((Task_FuncPtr) pFunc, &taskParams, NULL);

    if (pHandle->task == NULL) {
        retStatus = RPE_E_UTILS_TASK_CREATE;
        goto EXIT;
    }

    /* Task was successfully created */
    pHandle->isCreated = TRUE;

    *pTask = pHandle;

EXIT:
    if ((RPE_S_SUCCESS != retStatus) && (NULL != pHandle)) {
        if (NULL != pHandle->stackPtr)
            Memory_free (NULL, pHandle->stackPtr, pHandle->stackSize);
        Memory_free (NULL, pHandle, sizeof (Utils_Task));
    }

    return retStatus;
}

static int32_t Utils_taskDelete (
    Utils_Ptr                   pTask)
{
    int32_t                     retStatus = RPE_S_SUCCESS;
    Utils_Task                 *pHandle = (Utils_Task *) pTask;
    uint32_t                    uSleepTime = TASK_MIN_WAIT_TIME;

    if (NULL == pHandle) {
        retStatus = RPE_E_INVALIDARG;
        goto EXIT;
    }

    if (pHandle->isCreated != TRUE) {
        retStatus = RPE_E_UTILS_ERR_UNKNOWN;
        goto EXIT;
    }
    
    /*Check the status of the Task */
    while (Task_Mode_TERMINATED != Task_getMode (pHandle->task)) {

        Task_sleep (Utils_getTicks (uSleepTime));

        uSleepTime <<= 1;
        if (uSleepTime >= TASK_MAX_WAIT_TIME) {
            retStatus = RPE_E_UTILS_ERR_UNKNOWN;
            goto EXIT;
        }
    }
    Task_delete (&(pHandle->task));

    Memory_free (NULL, pHandle->stackPtr, pHandle->stackSize);
    Memory_free (NULL, pHandle, sizeof (Utils_Task));

EXIT:
    return retStatus;
}

void Utils_taskExit (
    Utils_Ptr                   pTask)
{
    Utils_Task                 *pHandle = (Utils_Task *) pTask;
    UInt32                      cookie;

    if (NULL == pHandle) {
        goto EXIT;
    }
    if (pHandle->isCreated != TRUE) {
        goto EXIT;
    }
    pHandle->next = NULL;

    cookie = Hwi_disable ();

    if (NULL == Utils_exitingTaskQue.head) {
        Utils_exitingTaskQue.head = Utils_exitingTaskQue.tail = pHandle;
    }
    else {
        Utils_exitingTaskQue.tail->next = pHandle;
        Utils_exitingTaskQue.tail = pHandle;
    }

    Hwi_restore (cookie);

    Semaphore_post (Rpe_monitorTaskSem);

    Task_exit ();

EXIT:
    return;
}

int32_t Utils_initMonitorTask ()
{
    int32_t                     status;
    Semaphore_Params            params;

    /*Initialize semaphore params with default values */
    Semaphore_Params_init (&params);
    params.mode = Semaphore_Mode_COUNTING;

    if ((Rpe_monitorTaskSem = Semaphore_create (0, &params, NULL)) == NULL) {
        status = RPE_E_OS_SEM_CREATE;
        goto Exit;
    }

    /* Create the monitor task */
    status = Utils_taskCreate (&Rpe_monitorTaskHandle,
                               Rpe_monitorTaskEntry,
                               0, NULL,
                               Rpe_monitorTaskConfig.stackSize,
                               Rpe_monitorTaskConfig.osPriority,
                               Rpe_monitorTaskConfig.taskName);

    if (RPE_S_SUCCESS != status) {
        status = RPE_E_OS_TASK_CREATE;
        goto Exit;
    }

    Rpe_monitortaskExitFlag = FALSE;

    status = RPE_S_SUCCESS;

Exit:
    return (status);
}

void Utils_exitMonitorTask (
    )
{
    Rpe_monitortaskExitFlag = TRUE;

    Semaphore_post (Rpe_monitorTaskSem);

    return;
}

Utils_Task *_Utils_removeTaskFromQueue ()
{
    Utils_Task                 *pHandle = NULL;
    UInt32                      cookie;

    cookie = Hwi_disable ();

    if (NULL != Utils_exitingTaskQue.head) {

        pHandle = Utils_exitingTaskQue.head;

        Utils_exitingTaskQue.head = pHandle->next;

        if (NULL == Utils_exitingTaskQue.head)
            Utils_exitingTaskQue.tail = NULL;
    }

    Hwi_restore (cookie);

    return (pHandle);
}

void Rpe_monitorTaskEntry (
    uint32_t                    argc,
    Utils_Ptr                   argv)
{
    Utils_Task                 *pHandle = NULL;

    for (;;) {
        Semaphore_pend (Rpe_monitorTaskSem, BIOS_WAIT_FOREVER);

        while ((pHandle = _Utils_removeTaskFromQueue ()) != NULL) {
            Utils_taskDelete (pHandle);
        }

        if (TRUE == Rpe_monitortaskExitFlag)
            Utils_taskExit (Rpe_monitorTaskHandle);        /* task exits */
    }
}

void Utils_deinitMonitorTask ()
{
    /* Signal monitor task to exit */
    Utils_exitMonitorTask ();

    /* Delete monitor task */
    Utils_taskDelete (Rpe_monitorTaskHandle);

    /* Delete semaphore used by monitor task */
    Semaphore_delete (&Rpe_monitorTaskSem);
}
