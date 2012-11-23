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
#include <ti/xdais/dm/isphdec1.h>
#include <ti/syslink/utils/IHeap.h>
#include <ti/syslink/utils/Memory.h>
#include <ti/ipc/SharedRegion.h>
#include <ti/sdo/codecs/aaclcdec/iaacdec.h>
#include <ti/xdais/dm/iauddec1.h>
#include <alsa/asoundlib.h>

/*-------------------------program files --------------------------------------*/
#include "ti/rpe.h"
#include "system_init.h"
#include "ringbuffer.h"

/*******************************************************************************
 * PRIVATE DECLARATIONS Defined here, used only here
 ******************************************************************************/

/*--------------------------- macros  ----------------------------------------*/
#define MAX_BUF_SIZE            1024
#define MAX_OUTPUT_BUFFER_SIZE (4096)
#define INPUT_RING_BUFFER_SIZE (128*1024-1)
#define MAX_FILE_STRING_SIZE    150
#define MAX_NO_OF_IO_BUFFERS    1

/**
 *******************************************************************************
 * @func             writeToFile
 * @brief            Writes the output of AAC decoder to file
 * @param[in]        outputFile   File pointer to output file.
 * @param[in]        out          Pointer to output buffer to be written to file.
 * @param[in]        status       status structure of AAC decoder.
 *******************************************************************************
*/

static void writeToFile ( FILE *outputFile, uint8_t *out, IAUDDEC1_Status *status )
{
    uint32_t bytesPerSample = (status->outputBitsPerSample >> 3);

    if(status->channelMode == IAUDIO_1_0)
    {
        fwrite(out, bytesPerSample, status->numSamples, outputFile);
    }
    else 
    {
        if(status->pcmFormat == IAUDIO_INTERLEAVED)
        {
          fwrite(out, bytesPerSample, status->numSamples*2, outputFile);
        }
        else
        { 
            uint32_t i;
            uint8_t * outL = (uint8_t *) out;
            uint8_t * outR = (uint8_t *)((uint8_t *) out + 
                                         (status->numSamples * bytesPerSample)) ;
            for(i=0;i<(status->numSamples * bytesPerSample); i+= bytesPerSample)
            {
              /*--------------------------------------------------------------*/
              /* Write the left anf right channels                            */
              /*--------------------------------------------------------------*/
              fwrite(&outL[i], bytesPerSample, 1, outputFile);
              fwrite(&outR[i], bytesPerSample, 1, outputFile);
            }
        }
    }
}

/* Handle for the PCM device */
static snd_pcm_t *playback_handle = NULL;
static snd_pcm_hw_params_t *hw_params;

/**
 *******************************************************************************
 * @func             configure_audio_drv
 * @brief            Configures the ALSA playback driver
 * @param[in]        rate   sample rate.
 *******************************************************************************
*/

static int configure_audio_drv(unsigned int rate)
{
    int err;
    int exact_rate;
    /* Playback stream */
    snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
    /* This structure contains information about the hardware and can be
    used to specify the configuration to be used for */
    /* the PCM stream. */

    /* Name of the PCM device, like plughw:0,0 */
    /* The first number is the number of the soundcard, the second number
    is the number of the device. */
    static char *device = "default"; /* playback device */

    /* Open PCM. The last parameter of this function is the mode. */
    if ((err = snd_pcm_open (&playback_handle, device, stream, 0))< 0) 
    {
        printf("Could not open audio device\n");
        return(1);
    }

    /* Allocate the snd_pcm_hw_params_t structure on the stack. */
    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) 
    {
        fprintf (stderr, "cannot allocate hardware parameters (%s)\n",
        snd_strerror (err));
        return(1);
    }

    /* Init hwparams with full configuration space */
    if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) <0) 
    {
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n", 
        snd_strerror (err));
        return(1);
    }

    /* Set access type. */
    if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, 
                                     SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) 
    {
        fprintf (stderr, "cannot set access type (%s)\n", snd_strerror
        (err));
        return(1);
    }
    /* Set sample format */
    if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, 
                                       SND_PCM_FORMAT_S16_LE)) < 0) 
    {
        fprintf (stderr, "cannot set sample format (%s)\n", snd_strerror
        (err));
        return(1);
    }

    /* Set sample rate. If the exact rate is not supported by the
    hardware, use nearest possible rate. */
    exact_rate = rate;
    if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, 
                                                &rate, 0)) < 0) 
    {
        fprintf (stderr, "cannot set sample rate (%s)\n", snd_strerror
        (err));
        return(1);
    }
    if (rate != exact_rate) 
    {
        fprintf(stderr, "The rate %d Hz is not supported by the hardware.\n \
        ==> Using %d Hz instead.\n", rate, exact_rate);
    }

    /* Set number of channels */
    if ((err = snd_pcm_hw_params_set_channels (playback_handle,hw_params, 2)) < 0) 
    {
        fprintf (stderr, "cannot set channel count (%s)\n", snd_strerror
        (err));
        return(1);
    }
    /* Apply HW parameter settings to PCM device and prepare device. */
    if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) 
    {
        fprintf (stderr, "cannot set parameters (%s)\n", snd_strerror
        (err));
        return(1);
    }

    snd_pcm_hw_params_free (hw_params);

    if ((err = snd_pcm_prepare (playback_handle)) < 0) 
    {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
        snd_strerror (err));
        return(1);
    }

   return 0;
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
    if ((err = snd_pcm_close (playback_handle))< 0) 
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

void Aac_Decoder_example (FILE *inlist)
{
    uint8_t                     *bufPtr = NULL;
    uint8_t                     *inputData = NULL;
    uint16_t                    *outputData = NULL;
    
    /* Heap to allocate input/output buffers */
    IHeap_Handle                heap = NULL;
    
    /* Decoder Create Parameter */
    IAUDDEC1_Params             decParams = {0};
    
    /* RPE component attribute structure used in ceate call */
    Rpe_Attributes              instAttr = {0};
    
    /* Client handle from create call */
    Rpe_ClientHandle            clientHandle = NULL;
    Rpe_CallDescHandle          processCallDesc = NULL, /* Process call desc */
                                controlCallDesc = NULL; /* Control call desc */
       
    /* Engaine call status */       
    int32_t                     status;                 

    /* Pointers to decoder control call parameters located in call decriptor */
    IAUDDEC1_Cmd                *cmdId;
    IAUDDEC1_DynamicParams      *decDynParams = NULL;
    IAUDDEC1_Status             *decStatus = NULL;

    /* Pointers to decoder process call parameters located in call decriptor */
    XDM1_BufDesc                *inBufDesc = NULL;
    XDM1_BufDesc                *outBufDesc = NULL;
    IAUDDEC1_InArgs             *decInArgs = NULL;
    IAUDDEC1_OutArgs            *decOutArgs = NULL;
    
    uint32_t                    minInputBufSize, 
                                maxOutputBufSize;

    /* Input bit-stream properties */
    int32_t                     desiredChannelMode;
    int32_t                     playback;
    char                        inputFileName [MAX_FILE_STRING_SIZE];
    char                        outputFileName [MAX_FILE_STRING_SIZE];
 
    int32_t                     frameCnt = 0;    /* Current frame count */
    
    /* FILE pointer for input file, output file */
    FILE                        *inputFile= NULL, 
                                *outputFile= NULL;
    
    /* Application ring buffer to read input bit-stream */
    RingBuffer_Params           rbParams = {0};
    RingBuffer_Object           *rbHandle = NULL;
    uint32_t                    bytesInBuf;

    uint32_t                    err;
   
    while (fscanf (inlist, "%d", &playback)     
        && fscanf (inlist, "%d", &desiredChannelMode)
        && fscanf (inlist, "%s", inputFileName)
        && fscanf (inlist, "%s", outputFileName))
    {        
        if (NULL == (inputFile = fopen(inputFileName,"rb")))  
        {
            printf("Error in opening input file\n");
            continue;
        }

        if (!playback)
        {
            if (NULL == (outputFile = fopen (outputFileName, "wb")))
            {
                printf ("Error opening out file %s.....proceeding to next file.\n",
                        outputFileName);
            }
        }
        
        /*-------------------------------------------------------------------*/
        /* Set AAC decoder create time parameters                            */
        /*-------------------------------------------------------------------*/
        decParams.size           = sizeof (IAUDDEC1_Params);
        decParams.outputPCMWidth = 16;
        decParams.pcmFormat      = IAUDIO_INTERLEAVED;
        decParams.dataEndianness = XDM_LE_16;

        /*--------------------------------------------------------------------*/
        /* Set inst attributes                                                */
        /*--------------------------------------------------------------------*/
        instAttr.priority            = RPE_PROCESSING_PRIORITY_MEDIUM;
        instAttr.inBufCpuAccessMode  = RPE_CPU_ACCESS_MODE_WRITE;
        instAttr.outBufCpuAccessMode = RPE_CPU_ACCESS_MODE_READ;
  
        status = Rpe_create ("AAC_ADEC_TI", 
                                &instAttr, 
                                &decParams, 
                                &clientHandle);
        if (RPE_S_SUCCESS != status)
        {
            printf ("Rpe_create failed, status: %d\n", status);
            return;
        }

        status = Rpe_acquireCallDescriptor (clientHandle, 
                                            RPE_CALL_DESC_CONTROL,
                                            &controlCallDesc,
                                            &cmdId, 
                                            &decDynParams,
                                            &decStatus);
        if (RPE_S_SUCCESS != status)
        {
            printf ("Rpe_acquireCallDescriptor failed, status: %d\n", status);
            return;
        }
        
        /*----------------------------------------------------------------------*/
        /* Call control api using XDM_GETBUFINFO to get I/O buffer requirements */
        /*----------------------------------------------------------------------*/

        *cmdId = XDM_GETBUFINFO;
        decDynParams->size = sizeof(*decDynParams);
        decStatus->size    = sizeof(*decStatus);

        status = Rpe_control (controlCallDesc);
        if (RPE_S_SUCCESS != status)
        {
            printf ("Rpe control call XDM_GETBUFINFO failed, status: %d\n", status);
            return;
        }
        
        status = Rpe_acquireCallDescriptor (clientHandle, 
                                            RPE_CALL_DESC_PROCESS,
                                            &processCallDesc,
                                            &inBufDesc, 
                                            &outBufDesc,
                                            &decInArgs, 
                                            &decOutArgs);
        if (RPE_S_SUCCESS != status)
        {
            printf ("Rpe_acquireCallDescriptor failed, status: %d\n", status);
            return;
        }
                                             
        inBufDesc->numBufs = decStatus->bufInfo.minNumInBufs;
        inBufDesc->descs[0].bufSize = minInputBufSize 
                                    = decStatus->bufInfo.minInBufSize[0];
        outBufDesc->numBufs = decStatus->bufInfo.minNumOutBufs;
        outBufDesc->descs[0].bufSize = decStatus->bufInfo.minOutBufSize[0];

        /*--------------------------------------------------------------------*/
        /* Allocate the I/O buffers from shared region                        */
        /*--------------------------------------------------------------------*/
        heap = SharedRegion_getHeap(IPC_SR_FRAME_BUFFERS_ID);

        inputData = (uint8_t*) Memory_alloc (heap, INPUT_RING_BUFFER_SIZE, 128, NULL);

        if (inputData == NULL) 
        {
            printf ("Allocation Failed for inputData \n");
            return;
        }
        
        bufPtr = inputData;
        
        maxOutputBufSize = MAX_OUTPUT_BUFFER_SIZE * sizeof (uint16_t);
        outputData = (uint16_t*) Memory_alloc (heap, maxOutputBufSize, 128, NULL);

        if (outputData == NULL) 
        {
            printf ("Allocation Failed for outputData \n");
            return;
        }

        inBufDesc->descs[0].buf  = (XDAS_Int8*)inputData;
        outBufDesc->descs[0].buf = (XDAS_Int8*)outputData;

        *cmdId = XDM_SETDEFAULT;
        decDynParams->size = sizeof(*decDynParams);
        decStatus->size    = sizeof(*decStatus);

        status = Rpe_control (controlCallDesc);
        if (RPE_S_SUCCESS != status)
        {
            printf ("Rpe control call XDM_SETDEFAULT failed, status: %d\n", status);
            return;
        }

        /*-------------------------------------------------------------------*/
        /* Call control api using XDM_SETPARAMS to set run-time parameters   */
        /*-------------------------------------------------------------------*/
  
        *cmdId = XDM_SETPARAMS;
        decDynParams->size = sizeof(*decDynParams);
        decStatus->size    = sizeof(*decStatus);
        decDynParams->downSampleSbrFlag = 0;
        
        status = Rpe_control (controlCallDesc);
        if (RPE_S_SUCCESS != status)
        {
            printf ("Rpe control call XDM_SETPARAMS failed, status: %d\n", status);
            return;
        }

        decInArgs->size    = sizeof(*decInArgs);
        decInArgs->numBytes = 0;
        decInArgs->lfeFlag = 0;
        decInArgs->desiredChannelMode = desiredChannelMode;

        decOutArgs->size   = sizeof(*decOutArgs);
        decOutArgs->bytesConsumed = 0;
        decOutArgs->extendedError = XDM_EOK;

        /*-------------------------------------------------------------------*/
        /* Update the ring buffer data structure                             */
        /*-------------------------------------------------------------------*/
        rbParams.bufferSize     = INPUT_RING_BUFFER_SIZE;
        rbParams.lowWaterMark   = minInputBufSize;
        rbParams.fpin           = inputFile;
        rbParams.buffer         = inputData;
  
        if (RingBuffer_create(&rbParams, &rbHandle) < 0) 
        {
            fprintf(stderr, "Error: Failed to create ring buffer\n");
            exit(1);
        }

        for ( ; ; ) 
        {
            if (RingBuffer_fill (rbHandle, &bytesInBuf, &inputData) == FALSE) 
            {
                break;
            }
            inBufDesc->descs[0].buf = (XDAS_Int8*)inputData;

            if (bytesInBuf > minInputBufSize)
            {
                decInArgs->numBytes = minInputBufSize;
                inBufDesc->descs[0].bufSize = minInputBufSize;
            }
            else
            {
                decInArgs->numBytes = bytesInBuf;
                inBufDesc->descs[0].bufSize = bytesInBuf;
            } 

            /*---------------------------------------------------------------*/
            /* Process call to decode the frame                              */
            /*---------------------------------------------------------------*/
            status = Rpe_process (processCallDesc);
            if (RPE_S_SUCCESS != status)
            {
                printf ("Rpe process call failed, status: %d\n", status);
                
                if ( decOutArgs->extendedError )
                {
                    printf ("AAC decoder failed error %0x to decode frame %d \n", 
                            (uint32_t) decOutArgs->extendedError, frameCnt);
                         
                    if (XDM_ISFATALERROR (decOutArgs->extendedError))
                    {
                        goto end;
                    }
                }
            }
       
            RingBuffer_setBytesConsumed (rbHandle, decOutArgs->bytesConsumed);
     
            if ( frameCnt == 0)
            {
                *cmdId = XDM_GETSTATUS;
                status = Rpe_control (controlCallDesc);    
                if (RPE_S_SUCCESS != status)
                {
                    printf ("Rpe control call XDM_GETSTATUS failed, status: %d\n", status);
                    return;
                }            
            }
     
            /*---------------------------------------------------------------*/
            /* Write output to File                                          */
            /* Skip first two blocks so output is time aligned with input    */
            /*---------------------------------------------------------------*/
            if((frameCnt > 1) && (decStatus->validFlag == 1))
            {
                if (!playback)
                { 
                    if (frameCnt == 2)
                    {
                        printf("\n Writing to file ...\n");
                    }
                    writeToFile(outputFile, 
                              (uint8_t*) outBufDesc->descs[0].buf,  decStatus);
                }
                else
                {
                    int32_t retVal;
                    if (frameCnt == 2)
                    {
                        printf("\nPlayback through Audio Driver...\n");
                        retVal = configure_audio_drv(decOutArgs->sampleRate 
                                 >> (decOutArgs->channelMode - 1));
                        if(retVal)
                        {
                            printf("Audio driver configuration failed \n");
                            return;
                        }
                    }
                    if ((err = snd_pcm_writei (playback_handle, 
                                               outBufDesc->descs[0].buf,
                                               decStatus->numSamples)) !=
                                               decStatus->numSamples) 
                    {
                        printf("write to audio interface failed\n");
                    }
                }
            }
            frameCnt++;
        }

        printf ("\nCompleted Decoding %d frames of %s file.\n",frameCnt,
                inputFileName);
    
        frameCnt=0;
        if (!playback)
        {
            if (outputFile) 
            {
                fclose (outputFile);
            }
        }
        
        if (inputFile) 
        {
            fclose (inputFile);
        }
  
        if (playback)
        { 
            err = close_audio_drv();
            if (err)
                printf ("Closing audio driver failed \n");
        }

end:    err = RingBuffer_delete (rbHandle);
        if (err)
            printf ("Freeing ring buffer failed \n");
        
        Memory_free (heap, bufPtr, INPUT_RING_BUFFER_SIZE);
        Memory_free (heap, outputData, maxOutputBufSize);
        
        status = Rpe_delete (clientHandle);
        if (RPE_S_SUCCESS != status)
        {
            printf ("Rpe_delete failed, status: %d\n", status);
            return;
        }

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
    
    printf (" AAC decoder example \n");
    printf ("======================\n");

    if (argc < 3)
    {
        printf ("Please enter the correct format \n");
        printf ("\n \t\t aacdec_a8host_debug.xv5t -i configfile \n\n");
        exit (-1);
    }

    strncpy (configFileName,  argv[2], sizeof(configFileName));
  
    if (NULL == (inlist = fopen(configFileName, "rt")))  
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

    /* Calling the decoder component */
    Aac_Decoder_example (inlist);

    fclose (inlist);

    Rpe_deinit();

    System_procDeInit ();
  
    exit (0);
}
