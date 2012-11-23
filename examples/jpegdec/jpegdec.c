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
#define USE_CMEM 0
#define USE_SR   1
#define IOBUF_MEM USE_SR

/*******************************************************************************
*                             INCLUDE FILES
*******************************************************************************/

/* -------------------- system and platform files ----------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ti/Std.h>
#include <stdio.h>
#include <ti/sdo/codecs/jpegdec/ijpegdec.h>
#include <ti/xdais/dm/iimgdec1.h>

#if  ( IOBUF_MEM == USE_SR )
  #include <ti/syslink/utils/IHeap.h>
  #include <ti/syslink/utils/Memory.h>
  #include <ti/ipc/SharedRegion.h>
#elif ( IOBUF_MEM == USE_CMEM )
  #include <ti/sdo/linuxutils/cmem/include/cmem.h>
#endif /*( IOBUF_MEM == USE_CMEM )*/

/*-------------------------program files --------------------------------------*/
#include "ti/rpe.h"
#include "system_init.h"


/*******************************************************************************
 * PRIVATE DECLARATIONS Defined here, used only here
 ******************************************************************************/

/*--------------------------- macros  ----------------------------------------*/
#define MAX_WIDTH            (128)
#define MAX_HEIGHT           (1200)
#define MAX_INPUT_BUF_SIZE (921600)
#define MAX_OUTPUT_BUF_SIZE (MAX_WIDTH*MAX_HEIGHT*4)
#define MAX_FILE_STRING_SIZE 150
#define MAX_NO_OF_IO_BUFFERS 1

/**
 *******************************************************************************
 * @func             writeRawDataToFile
 * @brief            Writes the output of JPEG decoder to file
 * @param[in]        outputFile   File pointer to output file.
 * @param[in]        out          Pointer to output buffer to be written to file.
 * @param[in]        width        Width of the image.
 * @param[in]        height       Height of the image.
 *******************************************************************************
*/

void writeRawDataToFile ( FILE *outputFile, uint8_t *out , uint32_t width, 
                          uint32_t height )
{
    uint8_t*  tmpbuffer = NULL;
    uint32_t row, col;
    uint8_t  b, g,  r, a;

    tmpbuffer = (uint8_t*)out;
    for (row = 0; row<height; row++)
    {
        for (col = 0; col<width; col++)
        {
            b = *tmpbuffer++;
            g = *tmpbuffer++;
            r = *tmpbuffer++;
            a = *tmpbuffer++;
            fwrite (&r, 1, 1, outputFile);
            fwrite (&g, 1, 1, outputFile);
            fwrite (&b, 1, 1, outputFile);
        }
    }
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

void Jpeg_Decoder_example (int argc, char **argv)
{
    uint8_t                     *inputData = NULL;
    uint8_t                     *outputData = NULL;
    
    /* Decoder Create Parameter */
    IJPEGDEC_Params             decParams = {{0}};
    
    /* RPE component attribute structure used in ceate call */
    Rpe_Attributes              instAttr = {0};
    
    /* Client handle from create call */
    Rpe_ClientHandle            clientHandle = NULL;
    Rpe_CallDescHandle          processCallDesc = NULL, /* Process call desc */
                                controlCallDesc = NULL; /* Control call desc */
       
    /* Engaine call status */       
    int32_t                     status;                 

    /* Pointers to decoder control call parameters located in call decriptor */
    IIMGDEC1_Cmd                *cmdId;
    IJPEGDEC_DynamicParams      *decDynParams = NULL;
    IIMGDEC1_Status             *decStatus = NULL;

    /* Pointers to decoder process call parameters located in call decriptor */
    XDM1_BufDesc                *inBufDesc = NULL;
    XDM1_BufDesc                *outBufDesc = NULL;
    IJPEGDEC_InArgs             *decInArgs = NULL;
    IJPEGDEC_OutArgs            *decOutArgs = NULL;
    
    /* Input configuration file */
    char                        configFileName [MAX_FILE_STRING_SIZE];
    
    /* Input bit-stream properties */
    int32_t                     readLen;
    char                        inputFileName [MAX_FILE_STRING_SIZE];
    char                        outputFileName [MAX_FILE_STRING_SIZE];
 
    int32_t                     frameCnt = 0;    /* Current frame count */
    int32_t                     ImageWidth, ImageHeight, Scan, ChromaFormat, InputFormat, 
                                ResizeOption, displayWidth, RGB_Format, outImgRes, numAU, 
                                numMCU_row, x_org, y_org, x_length, y_length, alpha_rgb;

    /* FILE pointer for config file, input file, output file */
    FILE                        *inputFile= NULL, 
                                *outputFile= NULL, 
                                *inlist= NULL;
#if ( IOBUF_MEM == USE_SR ) 
    /* Heap to allocate input/output buffers */
    IHeap_Handle                heap = NULL;
#elif ( IOBUF_MEM == USE_CMEM )
    CMEM_AllocParams cmemParams = {
        CMEM_HEAP,
        CMEM_NONCACHED,
        128,
    };
    void* cmemInPhyAddr;
    void* cmemOutPhyAddr;
#endif /*( IOBUF_MEM == USE_CMEM )*/

    if (argc < 3)
    {
        printf ("Please enter the correct format \n");
        printf ("\n \t\t jpegdec_a8host_debug.xv5t -i configfile \n\n");
        return;
    }
  
    strncpy (configFileName,  argv[2], sizeof(configFileName));
  
    if (NULL == (inlist = fopen(configFileName, "rt")))  
    {
        printf("Error in opening input file list\n");
        return;
    }
  
    while (fscanf (inlist, "%d", &ImageWidth)     
       && fscanf (inlist, "%d", &ImageHeight)
       && fscanf (inlist, "%d", &Scan)
       && fscanf (inlist, "%d", &ChromaFormat)
       && fscanf (inlist, "%d", &InputFormat)
       && fscanf (inlist, "%d", &ResizeOption)
       && fscanf (inlist, "%d", &displayWidth)
       && fscanf (inlist, "%d", &RGB_Format)
       && fscanf (inlist, "%d", &outImgRes)
       && fscanf (inlist, "%d", &numAU)
       && fscanf (inlist, "%d", &numMCU_row)
       && fscanf (inlist, "%d", &x_org)
       && fscanf (inlist, "%d", &y_org)
       && fscanf (inlist, "%d", &x_length)
       && fscanf (inlist, "%d", &y_length)
       && fscanf (inlist, "%d", &alpha_rgb)
       && fscanf (inlist, "%s", inputFileName)
       && fscanf (inlist, "%s", outputFileName))
    {        
        if (NULL == (inputFile = fopen(inputFileName,"rb")))  
        {
            printf("Error in opening input file %s\n", inputFileName);
            continue;
        }

        if (NULL == (outputFile = fopen (outputFileName, "wb")))
        {
            printf ("Error opening out file %s.....proceeding to next file.\n",
                    outputFileName);
        }

        /*-------------------------------------------------------------------*/
        /* Set IMGDEC decoder create time parameters                         */
        /*-------------------------------------------------------------------*/
        decParams.imgdecParams.size              = sizeof(IJPEGDEC_Params);
        decParams.imgdecParams.maxWidth          = 128;
        decParams.imgdecParams.maxHeight         = 1200;
        decParams.imgdecParams.dataEndianness    = XDM_BYTE;
        decParams.imgdecParams.forceChromaFormat = XDM_RGB;
        decParams.imgdecParams.maxScans          = 15; 
        decParams.progressiveDecFlag             = 1;
        decParams.outImgRes                      = 1;
        decParams.RGB_Output                     = 0;

        /*--------------------------------------------------------------------*/
        /* Set inst attributes                                                */
        /*--------------------------------------------------------------------*/
        instAttr.priority            = RPE_PROCESSING_PRIORITY_MEDIUM;
        instAttr.inBufCpuAccessMode  = RPE_CPU_ACCESS_MODE_WRITE;
        instAttr.outBufCpuAccessMode = RPE_CPU_ACCESS_MODE_READ;
  
        status = Rpe_create ("JPEG_IDEC_TI", 
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
        decDynParams->imgdecDynamicParams.size = sizeof(*decDynParams);
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
                                             
        inBufDesc->numBufs = 1;
        inBufDesc->descs[0].bufSize = MAX_INPUT_BUF_SIZE;
        outBufDesc->numBufs = 1;
        outBufDesc->descs[0].bufSize = MAX_OUTPUT_BUF_SIZE;

#if ( IOBUF_MEM == USE_SR ) 
        /*--------------------------------------------------------------------*/
        /* Allocate the I/O buffers from shared region                        */
        /*--------------------------------------------------------------------*/
        heap = SharedRegion_getHeap(IPC_SR_FRAME_BUFFERS_ID);
        inputData = (uint8_t*) Memory_alloc (heap, MAX_INPUT_BUF_SIZE, 128, NULL);
        if (inputData == NULL) 
        {
            printf ("Allocation Failed for inputData \n");
            goto EXIT;
        }
        
        outputData = (uint8_t*) Memory_alloc (heap, MAX_OUTPUT_BUF_SIZE, 128, NULL);

        if (outputData == NULL) 
        {
            printf ("Allocation Failed for outputData \n");
            goto EXIT;
        }
#elif ( IOBUF_MEM == USE_CMEM )
        inputData =  CMEM_alloc(MAX_INPUT_BUF_SIZE, &cmemParams);
        if (inputData == NULL) {
          printf("Failed to allocate CMEM buffer of size %d\n", MAX_INPUT_BUF_SIZE);
          goto EXIT;
        }
        cmemInPhyAddr = (void *) CMEM_getPhys(inputData);
        printf("cmem in at %p physical %p\n", inputData, cmemInPhyAddr);

        outputData = CMEM_alloc(MAX_OUTPUT_BUF_SIZE, &cmemParams);
        if (outputData == NULL) {
          printf("Failed to allocate CMEM buffer of size %d\n", MAX_OUTPUT_BUF_SIZE);
          goto EXIT;
        }
        cmemOutPhyAddr = (void *)CMEM_getPhys(outputData);
#endif /*( IOBUF_MEM == USE_CMEM )*/

        inBufDesc->descs[0].buf  = (XDAS_Int8*)inputData;
        outBufDesc->descs[0].buf = (XDAS_Int8*)outputData;

        *cmdId = XDM_SETDEFAULT;
        decDynParams->imgdecDynamicParams.size = sizeof(*decDynParams);
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
  
        readLen = fread (inputData, 1, inBufDesc->descs[0].bufSize, inputFile);

        *cmdId = XDM_SETPARAMS;
        decDynParams->imgdecDynamicParams.size = sizeof(IJPEGDEC_DynamicParams);
        decDynParams->alpha_rgb = 255;
        decDynParams->imgdecDynamicParams.numAU = XDM_DEFAULT;
        decDynParams->imgdecDynamicParams.decodeHeader = XDM_DECODE_AU;
        decDynParams->progDisplay = 0;
        decDynParams->resizeOption = 0;
        decDynParams->imgdecDynamicParams.displayWidth = XDM_DEFAULT; 
        decDynParams->RGB_Format = 1;
        decDynParams->numMCU_row = XDM_DEFAULT;
        decDynParams->x_org = 0;
        decDynParams->y_org = 0;
        decDynParams->x_length = 0;
        decDynParams->y_length = 0;
        decDynParams->frame_numbytes = readLen;
        
        decStatus->size    = sizeof(*decStatus);
        
        status = Rpe_control (controlCallDesc);
        if (RPE_S_SUCCESS != status)
        {
            printf ("Rpe control call XDM_SETPARAMS failed, status: %d\n", status);
            return;
        }
  
        decInArgs->imgdecInArgs.size      = sizeof(IJPEGDEC_InArgs);
        decInArgs->imgdecInArgs.numBytes  = readLen;
        
        decOutArgs->imgdecOutArgs.size          = sizeof(IJPEGDEC_OutArgs);
        decOutArgs->imgdecOutArgs.bytesConsumed = 0;
        decOutArgs->imgdecOutArgs.extendedError = XDM_EOK;

        for ( ; ; ) 
        {
            inBufDesc->descs[0].buf = (XDAS_Int8*)inputData;

            /*---------------------------------------------------------------*/
            /* Process call to decode the frame                              */
            /*---------------------------------------------------------------*/
            status = Rpe_process (processCallDesc);
            if (RPE_S_SUCCESS != status)
            {
                printf ( "Extended error %x\n", decOutArgs->imgdecOutArgs.extendedError);
                printf ("Rpe process call failed, status: %d\n", status);
                return;
            }
            /* NOTE:
             * Unmarshalling is not there in the RPE. Currently, this is to be 
             * done in the application.
             */
            inBufDesc->descs[0].buf  = (XDAS_Int8*)inputData;
            outBufDesc->descs[0].buf = (XDAS_Int8*)outputData;

            *cmdId = XDM_GETSTATUS;
            status = Rpe_control (controlCallDesc);    
            if (RPE_S_SUCCESS != status)
            {
                printf ("Rpe control call XDM_GETSTATUS failed, status: %d\n", status);
                return;
            }            
    
            /*---------------------------------------------------------------*/
            /* Write output to File                                          */
            /*---------------------------------------------------------------*/
            writeRawDataToFile(outputFile, 
                       (uint8_t*) outBufDesc->descs[0].buf,  ImageWidth, ImageHeight);
            frameCnt++;
            break; 
        }

        printf ("\nCompleted Decoding  %s file.\n", inputFileName);
    
        frameCnt=0;
        if (outputFile) 
        { 
            fclose (outputFile);
        }
        if (inputFile) 
        {
            fclose (inputFile);
        }

#if ( IOBUF_MEM == USE_SR )
        Memory_free ( heap, inputData,  MAX_INPUT_BUF_SIZE);        
        Memory_free ( heap, outputData, MAX_OUTPUT_BUF_SIZE);        
#elif  ( IOBUF_MEM == USE_CMEM )
        CMEM_free (inputData,  &cmemParams);
        CMEM_free (outputData, &cmemParams);
#endif /*( IOBUF_MEM == USE_CMEM )*/

        Rpe_delete (clientHandle);
        if (RPE_S_SUCCESS != status)
        {
            printf ("Rpe_create failed, status: %d\n", status);
            return;
        }

        printf ("Test completed\n");
    
    }
    fclose (inlist);
EXIT:
    return;
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
    printf (" JPEG decoder example \n");
    printf ("======================\n");


    /* Initializing */
    System_procInit ();

#if ( IOBUF_MEM == USE_CMEM )
    CMEM_init();
    printf ("CMEM_Init success\n");
#endif
    /* Calling the decoder component */
    Jpeg_Decoder_example (argc, argv);

    System_procDeInit ();
    exit (0);
}/* main */
/*jpegdec.c - EOF */
