
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

#include <assert.h>

#include "ti/rpe.h"
#include "ti/system_utils.h"

char *Rpe_getErrorString (int32_t errCode)
{
    switch (errCode) {

        case RPE_S_SUCCESS:
            return ("No error");

        case RPE_E_FAIL:
            return ("System failure");

        case RPE_E_INVALIDARG:
            return ("Invalid argument");

        case RPE_E_NULL_ARGPTR:
            return ("Null argument pointer");

        case RPE_E_ALREADYEXISTS:
            return ("Already exists");

        case RPE_E_CLIENT_NOT_FOUND:
            return ("Client not found");

        case RPE_E_INVALID_PROCESSOR_ID:
            return ("Invalid processor id");

        case RPE_E_INVALID_NUM_CONTROL_ARGS:
            return ("Invalid number of control call arguments");

        case RPE_E_INVALID_CONTROL_ARG_SIZE:
            return ("Invalid control call argumrnt size");

        case RPE_E_INVALID_NUM_PROCESS_ARGS:
            return ("Invalid number of process call argumrnts");

        case RPE_E_INVALID_PROCESS_ARG_SIZE:
            return ("Invalid process call argument size");

        case RPE_E_INVALID_RPE_CREATE_PARAM_SIZE:
            return ("Invalid create param size");

        case RPE_E_NULL_CLIENT_MARSHALL_FXN:
            return ("NULL client marshalling function");

        case RPE_E_NULL_CLIENT_UNMARSHALL_FXN:
            return ("NULL client unmarshalling function");

        case RPE_E_CLIENT_NO_MEMORY:
            return ("Memory allocation failure in client");

        case RPE_E_CLIENTDESC_NO_MEMORY:
            return ("No memory for call descriptor");

        case RPE_E_CLIENTDESC_NOTFREE:
            return ("Call descriptor not free");

        case RPE_E_IPC_READ_CONN:
            return ("Client unable to open input IPC connection");

        case RPE_E_IPC_WRITE_CONN:
            return ("Client unable to open outut IPC connection");

        case RPE_E_IPC_SEND:
            return ("Client unable to write to IPC connection");

        case RPE_E_IPC_RECV:
            return ("Client unable to read from IPC connection");

        case RPE_E_IPC_MSG:
            return ("Erroneous data received from IPC");

        case RPE_E_SHREG_NOTINIT:
            return ("Shared memory not initialized");

        case RPE_E_SHREG_INVALID:
            return ("Invalid shared memory");

        case RPE_E_SERVER_IPC_CONN:
            return ("Server unable to open input IPC connection");

        case RPE_E_SERVER_OS:
            return ("OS error on remote processor");

        case RPE_E_OS_TASK_CREATE:
            return ("OS task creation failure");

        case RPE_E_OS_SEM_CREATE:
            return ("OS demaphore creation failure");

        case RPE_E_SERVER_CREATE:
            return ("Server creation failed");

        case RPE_E_SERVER_CONTROL:
            return ("Server control call failed");

        case RPE_E_SERVER_PROCESS:
            return ("Server process call failed");

        case RPE_E_SERVER_DELETE:
            return ("Server delete failed");

        case RPE_E_SERVER_NOT_FOUND:
            return ("Server not found");

        case RPE_E_SERVER_NO_MEMORY:
            return ("Memory allocation failure in Server");

        case RPE_E_NULL_CREATE_FXN:
            return ("Null Server Create Function");

        case RPE_E_NULL_DELETE_FXN:
            return ("Null Server Delete Function");

        case RPE_E_NULL_CONTROL_FXN:
            return ("Null Server Control Function");

        case RPE_E_NULL_PROCESS_FXN:
            return ("Null Server Process Function");

        case RPE_E_NULL_SERVER_FXN:
            return ("Null Server Thread Entry Function");

        case RPE_E_INVALID_TASK_STACK_SIZE:
            return ("Invalid Task Stack Size");

        case RPE_E_INVALID_SERVER_HANDLE_SIZE:
            return ("Invalid Server Handle Size");

        case RPE_E_XDM_NULL_XDMFXNS:
            return ("Null Xdm function table pointer in server");

        case RPE_E_XDM_NULL_ALGALLOC_FXN:
            return ("NULL Xdais algAlloc Function");

        case RPE_E_XDM_NULL_ALGFREE_FXN:
            return ("NULL Xdais algFree Function");

        case RPE_E_XDM_NULL_ALGINIT_FXN:
            return ("NULL Xdais algInit Function");

        case RPE_E_XDM_INVALID_CREAT_PARAMS:
            return ("Invalid Create Params for Xdm algorithm");

        case RPE_E_XDM_ALGINIT:
            return ("Alginit call failed in Xdm algorithm");

        case RPE_E_XDM_NULL_ALG:
            return ("Null argument passed");

        case RPE_E_XDAIS_NO_MEMORY:
            return ("Xdais algorithm memory allocation failed");

        case RPE_E_XDM_NULL_PROCESS_PTR:
            return ("Null pointer to process function in Xdm algorithm");

        case RPE_E_XDM_NULL_PROCESS:
            return ("Null process function in Xdm algorithm");

        case RPE_E_XDM_NULL_CONTROL_PTR:
            return ("Null pointer to control function in Xdm algorithm");

        case RPE_E_XDM_NULL_CONTROL:
            return ("Null control function in Xdm algorithm");

        case RPE_TOO_MANY_ARGS:
            return ("Too many arguments");

        default:
            return ("Unknown error code");
    }
}
