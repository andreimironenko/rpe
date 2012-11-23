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
 *  @file  main.c
 *  @brief This file contains platform (A8) specific initializations and 
 *         the main () of the test application.
 *
 *  @rev 1.0
 *******************************************************************************
 */

/*******************************************************************************
*                             INCLUDE FILES
*******************************************************************************/

/* -------------------- system and platform files ----------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ti/Std.h>
#include <stdio.h>
#include <ti/xdais/dm/ispeech1_pcm.h>
#include <ti/xdais/dm/isphenc1.h>
#include <ti/syslink/utils/IHeap.h>
#include <ti/syslink/utils/Memory.h>
#include <ti/ipc/SharedRegion.h>
/* Get structure definitions for AACLC Audio Encoder Interface */
#include <ti/sdo/codecs/aaclcenc/imp4aacenc.h>
#include <ti/xdais/dm/iaudenc1.h>
#include <alsa/asoundlib.h>

/*-------------------------program files --------------------------------------*/
#include "ti/rpe.h"
#include "system_init.h"

/*******************************************************************************
 * PRIVATE DECLARATIONS Defined here, used only here
 ******************************************************************************/

/*--------------------------- macros  ----------------------------------------*/
#define MAX_BUF_SIZE            1024
#define MIN_INPUT_BUFFER_SIZE  (4096)
#define MAX_OUTPUT_BUFFER_SIZE (4096)
#define MAX_FILE_STRING_SIZE    150
#define MAX_NO_OF_IO_BUFFERS    1

typedef struct
{
    int16_t    compressionCode;
    int16_t    numberOfChannels ;
    int32_t    sampleRate ;
    int32_t    averageBytesPerSecond ;
    int16_t    blockAlign ;
    int16_t    bitsPerSample ;
    int16_t    extraFormatBytes ;
    /* extra stuff */
    int32_t    mainChunkSize ;

} SWavInfo;

typedef struct
{
    int8_t     chunkID[4] ;
    int32_t    chunkSize ;
    int32_t    dataOffset ;

} SChunk;

extern int32_t IsLittleEndian(void);
extern int32_t fread_EL(void *dst, int32_t size, int32_t nmemb, FILE *fp);
extern int32_t read_wave_header (FILE *fp, SWavInfo *wavInfo);

XDAS_Int8 channMap[7] = {-1, IAUDIO_1_0, IAUDIO_2_0, -1, -1, IAUDIO_3_2, IAUDIO_3_2};

/**
 *******************************************************************************
 * @func             fread_EL
 * @brief            Reads the input from file
 * @param[in]        dst     Pointer to store the input frame.
 * @param[in]        size    type of data to be read (byte, short, int).
 * @param[in]        nmemb   Number of bytes to be read.
 * @param[in]        fp      Pointer to input file.
 *******************************************************************************
*/
int32_t fread_EL(void *dst, int32_t size, int32_t nmemb, FILE *fp) 
{
    int16_t n, err;
    uint8_t *ptr;
    uint8_t tmp24[3];

    /* Enforce alignment of 24 bit data. */
    if (size == 3) 
    {
        ptr = (uint8_t *)dst;
        err = 0;
        for (n = 0; n < nmemb; n++) 
        {
            if ((err = fread (tmp24, 1, 3, fp)) != 3) 
            {
                return err;
            }
            *ptr++ = 0;
            *ptr++ = tmp24[0];
            *ptr++ = tmp24[1];
            *ptr++ = tmp24[2];
        }
        err = nmemb;
        size = sizeof (int32_t);
    } 
    else 
    {
        if ((err = fread (dst, size, nmemb, fp)) != nmemb) 
        {
            return err;
        }

    }
    return err;
}

/**
 *******************************************************************************
 * @func             read_wave_header
 * @brief            Reads wave header
 * @param[in]        fp         Pointer to input wave file.
 * @param[in]        wavInfo    Structure to hold wave header information.
 *******************************************************************************
*/
int32_t read_wave_header (FILE *fp, SWavInfo *wavInfo)
{
    SChunk fmt_chunk, data_chunk ;

    int32_t tmpSize ;
    int8_t tmpFormat[4] ;

    /* read RIFF-chunk */
    if(fread(tmpFormat, 1, 4, fp)!= 4)
    {
        return 2;  /* bad error "couldn't read RIFF_ID" */
    }
    if (strncmp("RIFF", (char *) tmpFormat, 4))
    {
        printf("RIFF Descriptor not found\n");
        return 2;
    }

    /* Read RIFF size. Ignored. */
    fmt_chunk.dataOffset = ftell(fp) ;
    fread_EL(&tmpSize, sizeof(int32_t), 1, fp);

    /********** From Here *************/ 
    /*! read WAVE-chunk */
    fmt_chunk.dataOffset = ftell(fp) ;
    if (fread(tmpFormat, sizeof(int8_t), 4, fp) !=4)
    {
        return 2;  /* bad error "couldn't read format" */
    }

    if (strncmp("WAVE", (char *) tmpFormat, 4)) 
    {
        printf("WAVE chunk ID not found.\n") ;
        return 2;
    }

    /* read format-chunk */
    fmt_chunk.dataOffset = ftell(fp) ;
    if (fread(fmt_chunk.chunkID, sizeof(int8_t), 4, fp) != 4)
    {
        return 3;  /* bad error "couldn't read format_ID" */
    }

    if (strncmp("fmt", (char *) fmt_chunk.chunkID, 3)) 
    {
        printf("fmt chunk format not found.\n") ;
        return 3;
    }

    /* should be 16 for PCM-format (uncompressed) */
    fmt_chunk.dataOffset = ftell(fp) ;
    fread_EL(&fmt_chunk.chunkSize, sizeof(int32_t), 1, fp);   
    fmt_chunk.dataOffset = ftell(fp) ;
    fseek (fp,fmt_chunk.chunkSize, SEEK_CUR) ;

    /* Search for "data" type chunk. Skip over other chunks. */
    do 
    {
        if (fread(data_chunk.chunkID, sizeof(int8_t), 4, fp) != 4) 
        {
            return 1;
        }
        fread_EL(&data_chunk.chunkSize, sizeof(int32_t), 1, fp);
        data_chunk.dataOffset = ftell(fp) ;
        // fseek (fp,data_chunk.chunkSize, SEEK_CUR) ;
    } while(!feof(fp) && (strncmp("data", (char *) data_chunk.chunkID, 4)) );

    fseek(fp,fmt_chunk.dataOffset, SEEK_SET) ;

    /* read  info */
    fread_EL(&(wavInfo->compressionCode), sizeof(int16_t), 1, fp);
    fread_EL(&(wavInfo->numberOfChannels), sizeof(int16_t), 1, fp);
    fread_EL(&(wavInfo->sampleRate), sizeof(int32_t), 1, fp);
    fread_EL(&(wavInfo->averageBytesPerSecond), sizeof(int32_t), 1, fp);
    fread_EL(&(wavInfo->blockAlign), sizeof(int16_t), 1, fp);
    fread_EL(&(wavInfo->bitsPerSample), sizeof(int16_t), 1, fp);

    /* Only for compressed audio, read extraFormatBytes */
    if (wavInfo->compressionCode > 0x07) 
    {
        fread_EL(&(wavInfo->extraFormatBytes), sizeof(int16_t), 1, fp);
    }

    /* now, file-pointer (fp) is set on beginning of sample data */
    fseek(fp,data_chunk.dataOffset, SEEK_SET) ;

    return 0 ;
}    

/* Handle for the PCM device */
static snd_pcm_t *record_handle = NULL;

/**
 *******************************************************************************
 * @func             configure_audio_drv
 * @brief            Configures the ALSA capture driver
 * @param[in]        channels      number of channels to capture.
 * @param[in]        samplerate    input sampling rate.
 * @param[in]        period_size   frame length to be capture.
 *******************************************************************************
*/

static int configure_audio_driver(int channels, int samplerate, int period_size)
{
    uint32_t eError;
    /* Playback stream */
    snd_pcm_stream_t stream = SND_PCM_STREAM_CAPTURE;

    /*
       This structure contains information about the hardware and can be
       used to specify the configuration to be used for
       the PCM stream.
     */
    snd_pcm_hw_params_t *hw_params;

    /*
       name of the device
     */
    static char *device = "default";

    int err, exact_rate;
    int dir, exact_period_size;
    exact_period_size = period_size;


    /*Open PCM. The last parameter of this function is the mode. */
    if ((err = snd_pcm_open(&record_handle, device, stream, 0)) < 0) {
        printf("Could not open audio device\n");
        return eError;
    }

    /* Allocate the snd_pcm_hw_params_t structure on the stack. */
    if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
        fprintf(stderr, "cannot allocate hardware parameters (%s)\n",
                snd_strerror(err));
        return eError;
    }

    /* Init hwparams with full configuration space */
    if ((err = snd_pcm_hw_params_any(record_handle, hw_params)) < 0) {
        fprintf(stderr,
                "cannot initialize hardware parameter structure (%s)\n",
                snd_strerror(err));
        return eError;
    }

    /* Set access type. */
    if ((err =
         snd_pcm_hw_params_set_access(record_handle, hw_params,
                                      SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf(stderr, "cannot set access type (%s)\n", snd_strerror(err));
        return eError;
    }

    /* Set sample format */
    if ((err =
         snd_pcm_hw_params_set_format(record_handle, hw_params,
                                      SND_PCM_FORMAT_S16_LE)) < 0) {
        fprintf(stderr, "cannot set sample format (%s)\n", snd_strerror(err));
        return eError;
    }

    /* Set sample rate. If the exact rate is not supported by the
       hardware, use nearest possible rate. */
    exact_rate = samplerate;
    if ((err =
         snd_pcm_hw_params_set_rate_near(record_handle, hw_params, (uint32_t *) &samplerate,
                                         0)) < 0) {
        fprintf(stderr, "cannot set sample rate (%s)\n", snd_strerror(err));
        return eError;
    }


    if (samplerate != exact_rate) {
        fprintf(stderr, "The rate %d Hz is not supported by the hardware. \
			Using %d Hz instead.\n", samplerate, exact_rate);
    }

    /* Set number of channels */
    if ((err =
         snd_pcm_hw_params_set_channels(record_handle, hw_params,
                                        channels)) < 0) {
        fprintf(stderr, "cannot set channel count (%s)\n", snd_strerror(err));
        return eError;
    }

    /* Set period size */
    if ((err =
         snd_pcm_hw_params_set_period_size_near(record_handle, hw_params,
                                      (snd_pcm_uframes_t *) &period_size, &dir)) < 0) {
        fprintf(stderr, "cannot set periodsize (%s)\n", snd_strerror(err));
        return eError;
    }
    if (period_size != exact_period_size) {
        fprintf(stderr, "using periodsize %d instead of %d \n", period_size,
                exact_period_size);
    }



    /* Apply HW parameter settings to PCM device and prepare device. */
    if ((err = snd_pcm_hw_params(record_handle, hw_params)) < 0) {
        fprintf(stderr, "cannot set parameters (%s)\n", snd_strerror(err));
        return eError;
    }

    snd_pcm_hw_params_free(hw_params);

    if ((err = snd_pcm_prepare(record_handle)) < 0) {
        fprintf(stderr, "cannot prepare audio interface for use (%s)\n",
                snd_strerror(err));
        return eError;
    }

    eError = 0;

    return eError;
}

/**
 *******************************************************************************
 * @func             close_audio_drv
 * @brief            Closes the ALSA playback driver
 *******************************************************************************
*/

static int close_audio_drv()
{
    int err;

    /* Close PCM */
    if ((err = snd_pcm_close (record_handle))< 0) 
    {
        printf("Could not close audio device\n");
        return(1);
    }

    return (0);
}

/** 
********************************************************************************
 *  @fn     main
 *  @brief  This function does the platform specific initialization. It then 
 *          calls the DSP Copy Example IL Client function. Finally, it performs
 *          platform specific de-initializations                               
 * 
 *  @param[in ]  argc  number of inpt arguements
 *  @param[in ]  argv  array of input arguements
 * 
 *  @returns none 
********************************************************************************
*/

void Aac_Encoder_example (FILE *inlist)
{
    uint16_t                    *inputData = NULL;
    uint8_t                     *outputData = NULL;
    
    /* Heap to allocate input/output buffers */
    IHeap_Handle                heap = NULL;
    
    /* Encoder Create Parameter */
    IAUDENC1_Params             encParams = {0};
    
    /* Pointers to decoder control call parameters located in call decriptor */
    IAUDENC1_DynamicParams      *encDynParams = NULL;
    IAUDENC1_Status             *encStatus = NULL;
    IAUDENC1_Cmd                *cmdId;
    
    /* Pointers to decoder process call parameters located in call decriptor */
    XDM1_BufDesc                *inBufDesc = NULL;
    XDM1_BufDesc                *outBufDesc = NULL;
    IAUDENC1_InArgs             *encInArgs = NULL;
    IAUDENC1_OutArgs            *encOutArgs = NULL;
    
    uint32_t                    minInputBufSize, 
                                maxOutputBufSize;
    
    /* RPE component attribute structure used in ceate call */
    Rpe_Attributes              instAttr = {0};
    
    /* Client handle from create call */
    Rpe_ClientHandle            clientHandle = NULL;
    Rpe_CallDescHandle          processCallDesc = NULL, /* Process call desc */
                                controlCallDesc = NULL; /* Control call desc */
    
    /* Engaine call status */       
    int32_t                     status;
    
    /* Input bit-stream properties */
    int32_t                     readBytes = 0;
    int32_t                     frameCnt = 0;
    int32_t                     recording, sampleRate, numberOfChannels;
    int32_t                     bytesperSample = 2, bitrate;
    char                        outputFileName[MAX_FILE_STRING_SIZE];
    char                        inputFileName [MAX_FILE_STRING_SIZE];
    SWavInfo                    fileInfo ;
    
    /* FILE pointer for input file, output file */
    FILE                        *inputFile= NULL, 
                                *outputFile= NULL;
                                
    uint32_t                    err;

    while (fscanf (inlist, "%d", &recording)     
        && fscanf (inlist, "%d", &numberOfChannels)
        && fscanf (inlist, "%d", &sampleRate)
        && fscanf (inlist, "%d", &bitrate)
        && fscanf (inlist, "%s", inputFileName)
        && fscanf (inlist, "%s", outputFileName))
    {        
        if (!recording)
        {
            if (NULL == (inputFile = fopen(inputFileName,"rb")))
            {
                printf ( "Error opening input file %s.....proceeding to next file.\n",
                       inputFileName );
            }
        }
        
        if (NULL == (outputFile = fopen (outputFileName, "wb")))
        {
            printf ( "Error opening out file %s.....proceeding to next file.\n",
                   outputFileName );
        }

        /*--------------------------------------------------------------------------*/
        /* Set default values to wave header before parsing                         */
        /*--------------------------------------------------------------------------*/
        fileInfo.bitsPerSample    = 16;
        fileInfo.numberOfChannels = numberOfChannels;
        fileInfo.sampleRate       = sampleRate;
  
        if (!recording) 
        {    
            printf ("Reading wave header \n");
            if (read_wave_header(inputFile, &fileInfo) )
            {
                printf("Error Reading WAVE header\n");
            }
        }
        
        /*--------------------------------------------------------------------*/
        /* Set AAC encoder create time parameters                             */
        /*--------------------------------------------------------------------*/
        encParams.size           = sizeof (IAUDENC1_Params);
        encParams.bitRate        = bitrate;
        encParams.ancFlag        = 0;
        encParams.channelMode    = channMap[fileInfo.numberOfChannels];
        encParams.dataEndianness = (XDAS_Int32)(XDM_LE_16);
        encParams.crcFlag        = 0;
        encParams.dualMonoMode   = 0;
        encParams.encMode        = IAUDIO_CBR;
        encParams.inputBitsPerSample = fileInfo.bitsPerSample;
        encParams.inputFormat    = IAUDIO_INTERLEAVED;
        encParams.lfeFlag        = 0;
        /* 800000 is the maximum bitrate for CBR but as we are not            */
        /* supporting VBR we need not assign its value.                       */
        encParams.maxBitRate     = 192000;
        encParams.sampleRate     = fileInfo.sampleRate;
        bytesperSample           = (fileInfo.bitsPerSample >> 3);

        /*--------------------------------------------------------------------*/
        /* Set RPE attributes                                                 */
        /*--------------------------------------------------------------------*/
        instAttr.priority            = RPE_PROCESSING_PRIORITY_MEDIUM;
        instAttr.inBufCpuAccessMode  = RPE_CPU_ACCESS_MODE_READ;
        instAttr.outBufCpuAccessMode = RPE_CPU_ACCESS_MODE_WRITE;
   
        status = Rpe_create ("AAC_AENC_TI", &instAttr, &encParams, &clientHandle);
        if (RPE_S_SUCCESS != status)
        {
            printf ("Rpe_create failed, status: %d\n", status);
            return;
        }
        
        status = Rpe_acquireCallDescriptor ( clientHandle, 
                                             RPE_CALL_DESC_CONTROL,
                                             &controlCallDesc,
                                             &cmdId, 
                                             &encDynParams,
                                             &encStatus);
        if (RPE_S_SUCCESS != status)
        {
            printf ("Rpe_acquireCallDescriptor failed, status: %d\n", status);
            return;
        }
        /*----------------------------------------------------------------------*/
        /* Call control api using XDM_GETBUFINFO to get I/O buffer requirements */
        /*----------------------------------------------------------------------*/
   
        *cmdId = XDM_GETBUFINFO;
        encDynParams->size = sizeof (IAUDENC1_DynamicParams);
        encStatus->size    = sizeof (IAUDENC1_Status);

        /*--------------------------------------------------------------------*/
        /* Set AAC encoder run time parameters                                */
        /*--------------------------------------------------------------------*/

        encDynParams->channelMode = encParams.channelMode;
        encDynParams->lfeFlag    = encParams.lfeFlag;
        encDynParams->sampleRate = encParams.sampleRate;
        encDynParams->bitRate = encParams.bitRate;
        encDynParams->dualMonoMode = encParams.dualMonoMode;
        encDynParams->inputBitsPerSample = encParams.inputBitsPerSample;
 
        status = Rpe_control (controlCallDesc);
        if (RPE_S_SUCCESS != status)
        {
            printf ("Rpe control call XDM_GETBUFINFO failed, status: %d\n", status);
            return;
        }

        status = Rpe_acquireCallDescriptor ( clientHandle, 
                                             RPE_CALL_DESC_PROCESS,
                                             &processCallDesc, 
                                             &inBufDesc, 
                                             &outBufDesc,
                                             &encInArgs, 
                                             &encOutArgs);
        if (RPE_S_SUCCESS != status)
        {
            printf ("Rpe_acquireCallDescriptor failed, status: %d\n", status);
            return;
        }
                                             
        inBufDesc->numBufs = encStatus->bufInfo.minNumInBufs;
        inBufDesc->descs[0].bufSize = encStatus->bufInfo.minInBufSize[0];
        outBufDesc->numBufs = encStatus->bufInfo.minNumOutBufs;
        outBufDesc->descs[0].bufSize = encStatus->bufInfo.minOutBufSize[0];

        /*--------------------------------------------------------------------*/
        /* Allocate the I/O buffers from shared region                        */
        /*--------------------------------------------------------------------*/
        minInputBufSize = MIN_INPUT_BUFFER_SIZE * sizeof (uint16_t);
        heap = SharedRegion_getHeap(IPC_SR_FRAME_BUFFERS_ID);
        inputData = (uint16_t*) Memory_alloc (heap, minInputBufSize, 128, NULL);
        
        if (inputData == NULL) 
        {
          printf ("Allocation Failed for inputData \n");
        }
        
        maxOutputBufSize = MAX_OUTPUT_BUFFER_SIZE;
        outputData = (uint8_t*) Memory_alloc (heap, maxOutputBufSize, 128, NULL);
        
        if (outputData == NULL) 
        {
          printf ("Allocation Failed for outputData \n");
        }

        inBufDesc->descs[0].buf = (XDAS_Int8*)inputData;
        outBufDesc->descs[0].buf = (XDAS_Int8*)outputData;

        *cmdId = XDM_SETDEFAULT;
        encDynParams->size = sizeof (IAUDENC1_DynamicParams);
        encStatus->size    = sizeof (IAUDENC1_Status);

        status = Rpe_control (controlCallDesc);
        if (RPE_S_SUCCESS != status)
        {
            printf ("Rpe control call XDM_SETDEFAULT failed, status: %d\n", status);
            return;
        }

        /*--------------------------------------------------------------------*/
        /* Call control api using XDM_SETPARAMS to set run-time parameters    */
        /*--------------------------------------------------------------------*/
   
        *cmdId = XDM_SETPARAMS;
        encDynParams->size = sizeof (IAUDENC1_DynamicParams);
        encStatus->size    = sizeof (IAUDENC1_Status);
        status = Rpe_control (controlCallDesc);
        if (RPE_S_SUCCESS != status)
        {
            printf ("Rpe control call XDM_SETPARAMS failed, status: %d\n", status);
            return;
        }

        encInArgs->size    = sizeof(IAUDENC1_InArgs);
        encInArgs->numInSamples = 0;
        encInArgs->ancData.buf = NULL;
        encInArgs->ancData.bufSize = 0;

        encOutArgs->size   = sizeof(IAUDENC1_OutArgs);
        encOutArgs->bytesGenerated = 0;
        encOutArgs->extendedError = XDM_EOK;

        while ( 1 ) 
        {
            if (!recording)
            {
                readBytes = fread (inputData, 
                                   1, 
                                   inBufDesc->descs[0].bufSize, 
                                   inputFile);
                               
                if (readBytes != inBufDesc->descs[0].bufSize)
                {
                   printf ("End of file reached \n");
                   break;
                }

                encInArgs->numInSamples = readBytes;
                encInArgs->numInSamples /= bytesperSample;
                encInArgs->numInSamples /= fileInfo.numberOfChannels;
            }
            else
            {
               uint32_t tFrames;
               
               if (frameCnt == 0)
               {
                   uint32_t frameLen, eError;
            
                   printf ("Configuring audio driver \n");
                   frameLen = (encStatus->bufInfo.minInBufSize[0]);
                   frameLen = ((frameLen << 3) / fileInfo.bitsPerSample);
                   frameLen = (frameLen / fileInfo.numberOfChannels);
                   eError   = configure_audio_driver(fileInfo.numberOfChannels,
                                      fileInfo.sampleRate, frameLen);
                   if (eError) {
                       printf("Audio driver configuration failed \n");
                       return;
                   }
               }
               
               tFrames = (inBufDesc->descs[0].bufSize << 3) / 
                           (fileInfo.numberOfChannels * fileInfo.bitsPerSample);
               
               readBytes = snd_pcm_readi(record_handle, inputData, tFrames);

               if (readBytes == -EPIPE) 
               {
                   /* EPIPE means overrun */
                   fprintf(stderr, "overrun occurred\n");
                   snd_pcm_prepare (record_handle);
               } 
               else if (readBytes < 0) 
               {
                   fprintf(stderr, "error from read: %s\n", snd_strerror(readBytes));
               } 
               else if (readBytes != tFrames) 
               {
                   fprintf(stderr, "short read, read %d frames\n", readBytes);
               }
               
               if (frameCnt == 1000)
                  break;

               encInArgs->numInSamples = readBytes;
            }
            
            status = Rpe_process (processCallDesc);
            if (RPE_S_SUCCESS != status)
            {
                printf ("Rpe process call failed, status: %d\n", status);
                if (encOutArgs->extendedError )
                {
                    printf("AAC encoder failed error %0x to encode frame %d \n", 
                         (uint32_t) encOutArgs->extendedError, frameCnt);
                    if (XDM_ISFATALERROR(encOutArgs->extendedError)) 
                    {
                       *cmdId = XDM_RESET;
                        status = Rpe_control (controlCallDesc);
                        if (RPE_S_SUCCESS != status)
                        {
                            printf ("Rpe control call XDM_RESET failed, status: %d\n", status);
                            return;
                        }            
                    }
                }
            }
            *cmdId = XDM_GETSTATUS;
            status = Rpe_control (controlCallDesc);
            if (RPE_S_SUCCESS != status)
            {
                printf ("Rpe control call XDM_GETSTATUS failed, status: %d\n", status);
                return;
            }            
          

            /*------------------------------------------------------------------------*/
            /* Write output to File                                                   */
            /* Skip first two blocks so output is time aligned with input             */
            /*------------------------------------------------------------------------*/

            fwrite (outputData, 1, encOutArgs->bytesGenerated, outputFile);

            frameCnt++;
        }

        if (!recording)
        {
            printf ("\nCompleted Encoding %d frames of %s file.\n",frameCnt,
                  inputFileName);
       
            if (inputFile) 
            {
              fclose (inputFile);
            }
        }
        else
        {
            err = close_audio_drv();
            if (err)
                printf ("Closing audio driver failed \n");
        }
        frameCnt=0;
        if (outputFile) 
        { 
          fclose (outputFile);
        }
        
        Memory_free (heap, inputData, minInputBufSize);
        Memory_free (heap, outputData, maxOutputBufSize);
        
        Rpe_delete ( clientHandle );
        printf ("Test completed\n");
    }
}


/** 
********************************************************************************
 *  @fn     main
 *  @brief  This function does the platform specific initialization. It then 
 *          calls the DSP Copy Example IL Client function. Finally, it performs
 *          platform specific de-initializations                               
 * 
 *  @param[in ]  arg1  : Not used, Reserved for future use
 *  @param[in ]  arg2  : Not used, Reserved for future use
 * 
 *  @returns none 
********************************************************************************
*/

int main (int argc, char **argv)
{
    char configFileName [MAX_FILE_STRING_SIZE];
    FILE *inlist= NULL;
    int32_t status;
    
    printf (" AAC encoder example \n");
    printf ("======================\n");

    if (argc < 3)
    {
        printf ("Please enter the correct format \n");
        printf ("\n \t\t aacenc_a8host_debug.xv5t -i configfile \n\n");
        exit (-1);
    }

    strncpy (configFileName,  argv[2], sizeof(configFileName));
  
    if(!(inlist = fopen(configFileName, "rt")))    
    {
        printf("Error in opening input file list\n");
        exit (-1);
    }
  
    printf ("\nStarting threads\n"); 
    fflush (stdout);

    /* Initializing */
  
    System_procInit ();

    /*------------------------------------------------------------------------*/
    /* RPE API to initialize the client config array with configs available   */
    /* on server. This API needs to called only once by the application after */
    /* the firmware is loaded                                                 */
    /*------------------------------------------------------------------------*/
    status = Rpe_init ();
    if (RPE_S_SUCCESS != status)
    {
        printf ("Rpe_init failed, status: %d\n", status);
        exit (-1);
    }

    /* Calling the encoder component */
    Aac_Encoder_example (inlist);

    fclose(inlist);

    Rpe_deinit();

    System_procDeInit ();
  
    exit (0);
}
