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

#include <ti/Std.h>

/* XDM interface */
#include <ti/xdais/dm/xdm.h>

/* Get structure definitions for AACLC Audio Decoder Interface */
#ifdef RPE_AACDEC_ENABLE
#include <ti/sdo/codecs/aaclcdec/iaacdec.h>
#endif

#ifdef RPE_AACENC_ENABLE
#include <ti/sdo/codecs/aaclcenc/imp4aacenc.h>
#endif

#ifdef RPE_MP3DEC_ENABLE
#include <ti/sdo/codecs/mp3dec/imp3dec.h>
#endif

#ifdef RPE_JPEGDEC_ENABLE
#include <ti/sdo/codecs/jpegdec/ijpegdec.h>
#endif

#ifdef RPE_TESTCODEC_ENABLE
#include <ti/xdais/dm/isphdec1.h>
#endif

/* Get structure definitions for server configurations */
#include "ti/xdm_server.h"

#include "class_def.h"

#ifdef RPE_JPEGDEC_ENABLE
extern IIMGDEC1_Fxns JPEGDEC_TI_IJPEGDEC;
#endif

#ifdef RPE_MP3DEC_ENABLE
extern IMP3DEC_Fxns MP3DEC_TII_IMP3DEC;
#endif

#ifdef RPE_TESTCODEC_ENABLE
extern const ISPHDEC1_Fxns SPHDEC1COPY_TI_ISPHDEC1COPY;
#endif

const Rpe_ServerClassConfig TI_AUDDEC1_serverClassConfig = {
    .classId                    =   RPE_TI_IAUDDEC1_CLASS,
    .createFxn                  =   XdmServer_create,
    .deleteFxn                  =   XdmServer_delete,
    .controlFxn                 =   XdmServer_control,
    .processFxn                 =   XdmServer_process,
    .marshallProcessFxn         =   XdmServer_marshallXdm1BufDescArgs,
    .serverTaskEntry            =   RpeServer_defaultServerTask,
    .serverHandleSize           =   sizeof(XdmServer_ServerObj),
    .numControlArgs             =   3,
    .numProcessArgs             =   4
};

const Rpe_ServerClassConfig TI_AUDENC1_serverClassConfig = {
    .classId                    =   RPE_TI_IAUDENC1_CLASS,
    .createFxn                  =   XdmServer_create,
    .deleteFxn                  =   XdmServer_delete,
    .controlFxn                 =   XdmServer_control,
    .processFxn                 =   XdmServer_process,
    .marshallProcessFxn         =   XdmServer_marshallXdm1BufDescArgs,
    .serverTaskEntry            =   RpeServer_defaultServerTask,
    .serverHandleSize           =   sizeof(XdmServer_ServerObj),
    .numControlArgs             =   3,
    .numProcessArgs             =   4
};

const Rpe_ServerClassConfig TI_IMGDEC1_serverClassConfig = {
    .classId                    =   RPE_TI_IIMGDEC1_CLASS,
    .createFxn                  =   XdmServer_create,
    .deleteFxn                  =   XdmServer_delete,
    .controlFxn                 =   XdmServer_control,
    .processFxn                 =   XdmServer_process,
    .marshallProcessFxn         =   XdmServer_marshallXdm1BufDescArgs,
    .serverTaskEntry            =   RpeServer_defaultServerTask,
    .serverHandleSize           =   sizeof(XdmServer_ServerObj),
    .numControlArgs             =   3,
    .numProcessArgs             =   4
};

const Rpe_ServerClassConfig TI_TESTCODEC_serverClassConfig = {
    .classId                    =   RPE_TI_ISPHDEC1_CLASS,
    .createFxn                  =   XdmServer_create,
    .deleteFxn                  =   XdmServer_delete,
    .controlFxn                 =   XdmServer_control,
    .processFxn                 =   XdmServer_process,
    .marshallProcessFxn         =   XdmServer_marshallXdm1SingleBufDescArgs,
    .serverTaskEntry            =   RpeServer_defaultServerTask,
    .serverHandleSize           =   sizeof(XdmServer_ServerObj),
    .numControlArgs             =   3,
    .numProcessArgs             =   4
};

#ifdef RPE_AACDEC_ENABLE
const XdmServer_ServerConfig TI_AACDEC_serverConfig = {
    .serverConfig               = {
        .name                   =   "AAC_ADEC_TI",
        .classId                =   RPE_TI_IAUDDEC1_CLASS,
        .taskStackSize          =   1024 * 10,
        .inBufCpuAccessMode     =   RPE_ALLBUFS_CPU_ACCESS_MODE_READ,
        .outBufCpuAccessMode    =   RPE_ALLBUFS_CPU_ACCESS_MODE_WRITE,
        .sizeofCreateArgs           = sizeof(IAACDEC_Params),
        .sizeofControlArgs          = { sizeof(IAACDEC_Cmd),
                                        sizeof(IAACDEC_DynamicParams),
                                        sizeof(IAACDEC_Status)}, 
        .sizeofProcessArgs          = { sizeof(XDM1_BufDesc),
                                        sizeof(XDM1_BufDesc),
                                        sizeof(IAACDEC_InArgs),
                                        sizeof(IAACDEC_OutArgs)},
                                  },
    .xdmFxns                    = (XDM_Fxns *)&AACDEC_TII_IAACDEC.iauddec,
};
#endif

#ifdef RPE_AACENC_ENABLE
const XdmServer_ServerConfig TI_AACENC_serverConfig = {
    .serverConfig               = {
        .name                   =   "AAC_AENC_TI",
        .classId                =   RPE_TI_IAUDENC1_CLASS,
        .taskStackSize          =   1024 * 10,
        .inBufCpuAccessMode     =   RPE_ALLBUFS_CPU_ACCESS_MODE_READ,
        .outBufCpuAccessMode    =   RPE_ALLBUFS_CPU_ACCESS_MODE_WRITE,
        .sizeofCreateArgs           = sizeof(IMP4AACENC_Params),
        .sizeofControlArgs          = { sizeof(IMP4AACENC_Cmd),
                                        sizeof(IMP4AACENC_DynamicParams),
                                        sizeof(IMP4AACENC_Status)}, 
        .sizeofProcessArgs          = { sizeof(XDM1_BufDesc),
                                        sizeof(XDM1_BufDesc),
                                        sizeof(IMP4AACENC_InArgs),
                                        sizeof(IMP4AACENC_OutArgs)},
                                  },
    .xdmFxns                    = (XDM_Fxns *)&MP4AACENC_TIJ_IMP4AACENC.iaudenc,
};
#endif

#ifdef RPE_MP3DEC_ENABLE
const XdmServer_ServerConfig TI_MP3DEC_serverConfig = {
    .serverConfig               = {
        .name                   =   "MP3_ADEC_TI",
        .classId                =   RPE_TI_IAUDDEC1_CLASS,
        .taskStackSize          =   1024 * 10,
        .inBufCpuAccessMode     =   RPE_ALLBUFS_CPU_ACCESS_MODE_WRITE,
        .outBufCpuAccessMode    =   RPE_ALLBUFS_CPU_ACCESS_MODE_WRITE,
        .sizeofCreateArgs           = sizeof(IMP3DEC_Params),
        .sizeofControlArgs          = { sizeof(IMP3DEC_Cmd),
                                        sizeof(IMP3DEC_DynamicParams),
                                        sizeof(IMP3DEC_Status)}, 
        .sizeofProcessArgs          = { sizeof(XDM1_BufDesc),
                                        sizeof(XDM1_BufDesc),
                                        sizeof(IMP3DEC_InArgs),
                                        sizeof(IMP3DEC_OutArgs)},
                                  },
    .xdmFxns                    = (XDM_Fxns *)&MP3DEC_TII_IMP3DEC.iauddec,
};
#endif

#ifdef RPE_JPEGDEC_ENABLE
const XdmServer_ServerConfig TI_JPEGDEC_serverConfig = {
    .serverConfig               = {
        .name                   =   "JPEG_IDEC_TI",
        .classId                =   RPE_TI_IIMGDEC1_CLASS,
        .taskStackSize          =   1024 * 10,
        .inBufCpuAccessMode     =   RPE_ALLBUFS_CPU_ACCESS_MODE_READ,
        .outBufCpuAccessMode    =   RPE_ALLBUFS_CPU_ACCESS_MODE_WRITE,
        .sizeofCreateArgs           = sizeof(IJPEGDEC_Params),
        .sizeofControlArgs          = { sizeof(IIMGDEC1_Cmd),
                                        sizeof(IJPEGDEC_DynamicParams),
                                        sizeof(IIMGDEC1_Status)}, 
        .sizeofProcessArgs          = { sizeof(XDM1_BufDesc),
                                        sizeof(XDM1_BufDesc),
                                        sizeof(IJPEGDEC_InArgs),
                                        sizeof(IJPEGDEC_OutArgs)},
                                  },
    .xdmFxns                    = (XDM_Fxns *)&JPEGDEC_TI_IJPEGDEC,
};
#endif

#ifdef RPE_TESTCODEC_ENABLE
const XdmServer_ServerConfig TI_TESTCODEC_serverConfig = {
    .serverConfig               = {
        .name                   =   "SPHDEC1_COPY_TI",
        .classId                =   RPE_TI_ISPHDEC1_CLASS,
        .taskStackSize          =   1024 * 10,
        .inBufCpuAccessMode     =   RPE_ALLBUFS_CPU_ACCESS_MODE_READ,
        .outBufCpuAccessMode    =   RPE_ALLBUFS_CPU_ACCESS_MODE_WRITE,
        .sizeofCreateArgs           = sizeof(ISPHDEC1_Params),
        .sizeofControlArgs          = { sizeof(ISPHDEC1_Cmd),
                                        sizeof(ISPHDEC1_DynamicParams),
                                        sizeof(ISPHDEC1_Status)}, 
        .sizeofProcessArgs          = { sizeof(XDM1_SingleBufDesc),
                                        sizeof(XDM1_SingleBufDesc),
                                        sizeof(ISPHDEC1_InArgs),
                                        sizeof(ISPHDEC1_OutArgs)},
                                 },
    .xdmFxns                    = (XDM_Fxns *)&SPHDEC1COPY_TI_ISPHDEC1COPY,
};
#endif

const Rpe_ServerClassConfig Rpe_endServerClassConfig = {
    .classId                    =   RPE_CLASS_UNDEFINED
};

const Rpe_ServerClassConfig *Rpe_serverClassConfigArray[] = {
    & TI_AUDDEC1_serverClassConfig,
    & TI_AUDENC1_serverClassConfig,
    & TI_IMGDEC1_serverClassConfig,
    & TI_TESTCODEC_serverClassConfig,
    & Rpe_endServerClassConfig
};

const Rpe_ServerConfig Rpe_endServerConfig = {
    .name                       =   NULL
};

const Rpe_ServerConfig *Rpe_serverConfigArray[] =
{
#ifdef RPE_AACDEC_ENABLE
    (const Rpe_ServerConfig *) & TI_AACDEC_serverConfig,
#endif
#ifdef RPE_AACENC_ENABLE
    (const Rpe_ServerConfig *) & TI_AACENC_serverConfig,
#endif
#ifdef RPE_MP3DEC_ENABLE
    (const Rpe_ServerConfig *) & TI_MP3DEC_serverConfig,
#endif
#ifdef RPE_JPEGDEC_ENABLE
    (const Rpe_ServerConfig *) & TI_JPEGDEC_serverConfig,
#endif
#ifdef RPE_TESTCODEC_ENABLE
    (const Rpe_ServerConfig *) & TI_TESTCODEC_serverConfig,
#endif
    & Rpe_endServerConfig
};

const uint32_t Rpe_serverConfigCount = (sizeof(Rpe_serverConfigArray)
                                        / sizeof (Rpe_ServerConfig *)) - 1;
