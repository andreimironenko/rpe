# Makefile for dm814xbm app

-include app_properties.mk
-include ../../makerules/env.mk

APP_NAME = dm81xx
PLATFORM = ti814x-evm
SRCDIR = src ../common
INCDIR = . src ../common

# List all the external components/interfaces, whose interface header files 
#  need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = bios xdc ipc xdais syslink \
                             rpe ldrmemcfg systop

# List of all external libraries that need to be linked

ifeq ($(RPE_USEAACDEC), 1)
AACDECLIBPATH = $(aaclcdec_PATH)/packages/ti/sdo/codecs/aaclcdec/lib/aacdec_tii_lc_elf.l64P
endif
ifeq ($(RPE_USEAACENC), 1)
AACENCLIBPATH = $(aaclcenc_PATH)/packages/ti/sdo/codecs/aaclcenc/lib/mp4aacenc_tij_lc_elf.l67
endif
ifeq ($(RPE_USEMP3DEC), 1)
MP3DECLIBPATH = $(mp3dec_PATH)/packages/ti/sdo/codecs/mp3dec/lib/mp3dec_tii_l1l2l3_ELF.l64P
endif
ifeq ($(RPE_USEJPEGDEC), 1)
JPEGDECLIBPATH = $(jpegdec_PATH)/packages/ti/sdo/codecs/jpegdec/lib/jpegdec_ti.l64P
endif
ifeq ($(RPE_USETESTCODEC), 1)
TESTCODECLIBPATH = $(ROOTDIR)/lib/lib/c674/$(PROFILE_c6xdsp)/rpetests.ae674
endif

EXTLIBS_m3video =

EXTLIBS_m3vpss = 

EXTLIBS_a8host = 

EXTLIBS_c6xdsp = $(AACDECLIBPATH)\
                 $(AACENCLIBPATH)\
                 $(MP3DECLIBPATH)\
                 $(JPEGDECLIBPATH)\
                 $(TESTCODECLIBPATH) \
                 $(ROOTDIR)/lib/lib/c674/$(PROFILE_c6xdsp)/rpe.ae674 \
                 $(mcutils_PATH)/lib/lib/c674/debug/ldrmemcfg.ae674

# List all the components required by the application
COMP_LIST_m3video = 
COMP_LIST_m3vpss = 
COMP_LIST_a8host = 
COMP_LIST_c6xdsp = 

# XDC CFG File
XDC_CFG_FILE_m3vpss = 
XDC_CFG_FILE_m3video = 
XDC_CFG_FILE_c6xdsp = DspAppMain.cfg

# Common source files and CFLAGS across all platforms and cores
SRCS_COMMON =
CFLAGS_LOCAL_COMMON =

SRCS_m3vpss = 

CFLAGS_LOCAL_m3vpss = 

SRCS_m3video = 

CFLAGS_LOCAL_m3video = 

SRCS_a8host = 

CFLAGS_LOCAL_a8host = 

SRCS_c6xdsp = main_c674.c platform_c674.c sys_top_slave.c sysinit_bios.c
CFLAGS_LOCAL_c6xdsp = 

# Core/SoC/platform specific source files and CFLAGS
# Example: 
#   SRCS_<core/SoC/platform-name> = 
#   CFLAGS_LOCAL_<core/SoC/platform-name> =
ifeq ($(CORE),c6xdsp)
  CFLAGS_LOCAL_c6xdsp += -D_LOCAL_CORE_c6xdsp_
endif
ifeq ($(CORE),m3vpss)
  CFLAGS_LOCAL_m3vpss += -D_LOCAL_CORE_m3vpss_
endif
ifeq ($(CORE),m3video)
  CFLAGS_LOCAL_m3video += -D_LOCAL_CORE_m3video_
endif
ifeq ($(CORE),a8host)
  CFLAGS_LOCAL_a8host += -D_LOCAL_CORE_a8host_ 
endif

# Include common make files
include $(ROOTDIR)/makerules/common.mk

# OBJs and libraries are built by using rule defined in rules_<target>.mk 
#     and need not be explicitly specified here

# Nothing beyond this point
