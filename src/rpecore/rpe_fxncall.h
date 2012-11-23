
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
 *  @file       rpe_internal.h
 *
 *  @brief      Defines functions used internally in the framework.
 * 
 */

#ifndef RPE_INTERNAL_H
#define RPE_INTERNAL_H

/******************************************************************************
 * Internal functions defined in file rpe_fxncall.c
 ******************************************************************************/

/**
 *  @brief      This function retrives pointers to RPE call arguments
 *              present in a call descriptor. 
 *
 *  @param[in]  desc        Pointer to call descriptor.
 *  @param[out] argCnt      Number of arguments to an RPE call.
 *  @param[out] args        Array of argument pointers.
 *
 *  @retval     RPE_S_SUCCESS    Success
 */

void Rpe_getFunctionCallArgs (
    Rpe_CallDesc               *desc,
    uint8_t                    *argCnt,
    FArg                        args[]);


/**
 *  @brief      This function takes a function pointer, argumrnt count and an
 *              array of argument pointers and makes the function call after
 *              expanding the arguments.
 *
 *  @param[in]  fxnPtr       Function pointer of any type.
 *  @param[in]  argCnt       Number of arguments to function call.
 *  @param[in]  argv         Array of argument pointers.
 *
 *  @retval     RPE_S_SUCCESS    Success
 *
 *  @remarks    This function is used by both client and server.
 */

int32_t Rpe_makeFunctionCall (
    const Rpe_FxnPtr            fxnPtr,
    uint8_t                     argCnt,
    FArg                        argv[]);

#endif     /* RPE_INTERNAL_H */
