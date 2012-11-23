
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
 *  @file       rpe_fxncall.c
 *
 *  @brief      Implements generic utility functions to make function calls
 *              in RPE client and server.
 * 
 */

#include <ti/Std.h>
#include <ti/ipc/MessageQ.h>

#include "ti/rpe.h"
#include "ti/rpe_types.h"

#include "rpe_calldesc.h"
#include "rpe_fxncall.h"

/*===================== Local Type Definitions ==============================*/

/**
 *  @brief      Generic pointer to a function that takes one argument.
 */
typedef int32_t (*Rpe_FxnPtrA1) (FArg arg1);

/**
 *  @brief      Generic pointer to a function that takes two arguments.
 */
typedef int32_t (*Rpe_FxnPtrA2) (FArg arg1, FArg arg2);

/**
 *  @brief      Generic pointer to a function that takes three arguments.
 */
typedef int32_t (*Rpe_FxnPtrA3) (FArg arg1, FArg arg2, FArg arg3);

/**
 *  @brief      Generic pointer to a function that takes four arguments.
 */
typedef int32_t (*Rpe_FxnPtrA4) (FArg arg1, FArg arg2, FArg arg3, FArg arg4);

/**
 *  @brief      Generic pointer to a function that takes five arguments.
 */
typedef int32_t (*Rpe_FxnPtrA5) (FArg arg1, FArg arg2, FArg arg3, FArg arg4, 
                                 FArg arg5);

/**
 *  @brief      Generic pointer to a function that takes six arguments.
 */
typedef int32_t (*Rpe_FxnPtrA6) (FArg arg1, FArg arg2, FArg arg3, FArg arg4, 
                                 FArg arg5, FArg arg6);

/**
 *  @brief      Generic pointer to a function that takes seven arguments.
 */
typedef int32_t (*Rpe_FxnPtrA7) (FArg arg1, FArg arg2, FArg arg3, FArg arg4, 
                                 FArg arg5, FArg arg6, FArg arg7);

/*===================== Function Definitions ================================*/

/******************************************************************************
 *  ========== Rpe_getFunctionCallArgs() ==========
 *
 *  See rpe_internal.h
 *****************************************************************************/

void Rpe_getFunctionCallArgs (
    Rpe_CallDesc               *desc,
    uint8_t                    *argCnt,
    FArg                        args[])
{
    uint16_t                    i;
    char                       *argptr;
    
    /* Get pointer to the first argument */
    argptr = RPE_GET_FIRST_ARG_PTR (desc); 

    for (i = 0; i < desc->argCnt; ++i) {
        args[i] = argptr;            /* Store the current arg. pointer */
        argptr += desc->argSize[i];  /* Add arg. size to get the next arg. */
    }

    *argCnt = desc->argCnt;          /* Return argument count */

    return;
}

/******************************************************************************
 *  ========== Rpe_makeFunctionCall() ==========
 *
 *  See rpe_internal.h
 *****************************************************************************/

int32_t Rpe_makeFunctionCall (
    const Rpe_FxnPtr            fxnPtr,
    uint8_t                     argCnt,
    FArg                        argv[])
{
    int32_t                     status;

    switch (argCnt) {

        case 1:
            status = ((Rpe_FxnPtrA1) fxnPtr) (argv[0]);
            break;

        case 2:
            status = ((Rpe_FxnPtrA2) fxnPtr) (argv[0], argv[1]);
            break;

        case 3:
            status = ((Rpe_FxnPtrA3) fxnPtr) (argv[0], argv[1], argv[2]);
            break;

        case 4:
            status = ((Rpe_FxnPtrA4) fxnPtr) (argv[0], argv[1], argv[2],
                                              argv[3]);
            break;

        case 5:
            status = ((Rpe_FxnPtrA5) fxnPtr) (argv[0], argv[1], argv[2], 
                                              argv[3], argv[4]);
            break;
            
        case 6:
            status = ((Rpe_FxnPtrA6) fxnPtr) (argv[0], argv[1], argv[2], 
                                              argv[3], argv[4], argv[5]);
            break;
            
        case 7:
            status = ((Rpe_FxnPtrA7) fxnPtr) (argv[0], argv[1], argv[2], 
                                              argv[3], argv[4], argv[5],
                                              argv[6]);
            break;

        default:
            status = RPE_TOO_MANY_ARGS;
            break;
    }

    return (status);
}
