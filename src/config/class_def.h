
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
 *  @file       class_def.h
 *
 *  @brief      Defines a list of algorithm classes supported in the client.
 * 
 */

#ifndef CLASS_DEF_H
#define CLASS_DEF_H

#include <stdint.h>

/** Undefined class */
#define RPE_CLASS_UNDEFINED     0x0

/** Classes supported by current RPE XDM server */
#define RPE_TI_XDM_CLASS_BASE   0x10000

/** Audio decoders using XDM IAUDDEC1 interface */
#define RPE_TI_IAUDDEC1_CLASS   (RPE_TI_XDM_CLASS_BASE)

/** Audio encoders using XDM IAUDENC1 interface */
#define RPE_TI_IAUDENC1_CLASS   (RPE_TI_XDM_CLASS_BASE + 1)

/** Speech decoders using XDM ISPHDEC1 interface */
#define RPE_TI_ISPHDEC1_CLASS   (RPE_TI_XDM_CLASS_BASE + 2)

/** Speech encoders using XDM ISPHENC1 interface */
#define RPE_TI_ISPHENC1_CLASS   (RPE_TI_XDM_CLASS_BASE + 3)

/** Image decoders using XDM IIMGDEC1 interface */
#define RPE_TI_IIMGDEC1_CLASS   (RPE_TI_XDM_CLASS_BASE + 4)

#endif /* CLASS_DEF_H */

