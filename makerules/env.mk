# File: env.mk. This file contains all the paths and other ENV variables

#
# Module paths
#

# Directory where all internal software packages are located; typically 
#  those that are checked into version controlled repository. In this case all
#  the OMX components and SDK/OMX demo.
INTERNAL_SW_ROOT = $(ROOTDIR)/src

# Directory where all external (imported) software packages are located; typically 
#  those that are NOT checked into version controlled repository. In this case,
#  compiler tool chains, BIOS, XDC, Syslink, IPC, FC, CE, drivers, codecs, etc.
EXTERNAL_SW_ROOT = "EXTERNAL_SW_ROOT_UNDEFINED_"

# Destination root directory.
#   - specify the directory where you want to place the object, archive/library,
#     binary and other generated files in a different location than source tree
#   - or leave it blank to place then in the same tree as the source
DEST_ROOT = $(INTERNAL_SW_ROOT)/../lib

# Directory where example-apps/demos are located. By default, it resides along
#  with other source code. This can be over-ridden by specifying it in the 
#  command line.
EXAMPLES_ROOT = $(ROOTDIR)/examples

# Utilities directory. This is required only if the build machine is Windows.
#   - specify the installation directory of utility which supports POSIX commands
#     (eg: Cygwin installation or MSYS installation).
UTILS_INSTALL_DIR = c:/cygwin

# Set path separator, etc based on the OS
ifeq ($(OS),Windows_NT)
  PATH_SEPARATOR = ;
  UTILSPATH = $(UTILS_INSTALL_DIR)/bin/
else 
  # else, assume it is linux
  PATH_SEPARATOR = :
endif

#####################################################
# Codecs Paths
#####################################################

# AACDEC - AAC Decoder
aaclcdec_PATH = "aaclcdec_PATH_UNDEFINED_"
aaclcdec_INCLUDE = $(aaclcdec_PATH)/packages

# AACENC - AAC Encoder
aaclcenc_PATH = "aaclcenc_PATH_UNDEFINED_"
aaclcenc_INCLUDE = $(aaclcenc_PATH)/packages

# MP3DEC - MP3 Decoder
mp3dec_PATH = "mp3dec_PATH_UNDEFINED_"
mp3dec_INCLUDE = $(mp3dec_PATH)/packages

#
# <Integrator>: Add more codec paths here.
#

#####################################################
# Dependent Component Paths
#####################################################

# BIOS
bios_PATH = "bios_PATH_UNDEFINED_"
bios_INCLUDE = $(bios_PATH)/packages

# IPC
ipc_PATH = "ipc_PATH_UNDEFINED_"
ipc_INCLUDE = $(ipc_PATH)/packages

# XDAIS
xdais_PATH = "xdais_PATH_UNDEFINED_"
xdais_INCLUDE = $(xdais_PATH)/packages

# SYSLINK 
syslink_PATH = "syslink_PATH_UNDEFINED_"
syslink_INCLUDE = $(syslink_PATH)/packages

# XDC
xdc_PATH = "xdc_PATH_UNDEFINED_"
xdc_INCLUDE = $(xdc_PATH)/packages

# UIA
uia_PATH = "uia_PATH_UNDEFINED_"
uia_INCLUDE = $(uia_PATH)/packages

# EDMA 
edma_PATH = "edma_PATH_UNDEFINED_"
edma_INCLUDE = $(edma_PATH)/packages

# Linux Dev Kit
lindevkit_PATH = $(EXTERNAL_SW_ROOT)/../linux-devkit/arm-none-linux-gnueabi/usr
lindevkit_INCLUDE = $(lindevkit_PATH)/include

# PSP
ifeq ($(PLATFORM),ti816x-evm)
kernel_PATH = "kernel_PATH_UNDEFINED_"
kernel_INCLUDE = $(kernel_PATH)/src/kernel/linux-04.00.01.13/include $(kernel_PATH)/src/kernel/linux-04.00.01.13/arch/arm/include
else
kernel_PATH = "kernel_PATH_UNDEFINED_"
kernel_INCLUDE = $(kernel_PATH)/src/kernel/linux-04.01.00.06/include $(kernel_PATH)/src/kernel/linux-04.01.00.06/arch/arm/include
endif

# Media Controller Utils (for Systop and LDRMEMCFG). 
mcutils_PATH = "mcutils_PATH_UNDEFINED_"

# SYSTOP - module 
systop_RELPATH = $(mcutils_PATH)/src/sys_top
systop_PATH = $(systop_RELPATH)
systop_INCLUDE = $(systop_PATH)

# LDRMEMCFG - module
ldrmemcfg_RELPATH = $(mcutils_PATH)/src/ldrmemcfg
ldrmemcfg_PATH = $(ldrmemcfg_RELPATH)
ldrmemcfg_INCLUDE = $(ldrmemcfg_PATH)
ldrmemcfg_CORE_DEPENDENCY = yes
# ldrmemcfg_PKG_LIST = ldrmemcfg

# Audio (ALSA)
audio_INCLUDE = $(lindevkit_INCLUDE)

# RPE Framework source directory
rpe_PATH = $(INTERNAL_SW_ROOT)
rpe_LIBPATH = $(rpe_PATH)
include $(rpe_PATH)/component.mk

# LINUXUTILS
linuxutils_PATH = "linuxutils_PATH_UNDEFINED_"
linuxutils_INCLUDE = $(linuxutils_PATH)/packages

#
# Tools paths
#
# Cortex-M3

CODEGEN_PATH_M3 = $(EXTERNAL_SW_ROOT)/cgt470_4_9_0

# Cortex-A8
CODESOURCERY_PATH = "CODESOURCERY_PATH_UNDEFINED_"
CODEGEN_PATH_A8 = $(CODESOURCERY_PATH)

# DSP - Since same toolchain does not support COFF and ELF, there are two entries
#        This would go away when one version supports both formats
CODEGEN_PATH_DSP = $(EXTERNAL_SW_ROOT)/../dsp-devkit/cgt6x_7_3_1
CODEGEN_PATH_DSPELF = "CODEGEN_PATH_DSPELF_UNDEFINED_"

# Commands commonly used within the make files

RM = $(UTILSPATH)rm
RMDIR = $(UTILSPATH)rm -rf
MKDIR = $(UTILSPATH)mkdir
ECHO = @echo
# MAKE = $(UTILSPATH)make
EGREP = $(UTILSPATH)egrep
CP = $(UTILSPATH)cp
CHMOD = $(UTILSPATH)chmod

#
# XDC specific ENV variables
#

# XDC Config.bld file (required for configuro) ; Derives from top-level rpe_PATH
CONFIG_BLD_XDC_c674 = $(ROOTDIR)/examples/dm81xx/config.bld
CONFIG_BLD_XDC_m3 = $(rpe_PATH)/build/config.bld
CONFIG_BLD_XDC_a8 = $(rpe_PATH)/build/config_ca8.bld

XDCPATH = $(bios_PATH)/packages;$(ipc_PATH)/packages;$(syslink_PATH)/packages;$(xdc_PATH)/packages;$(rpe_PATH);.;$(aaclcdec_PATH)/packages;$(mp3dec_PATH)/packages;$(aaclcenc_PATH)/packages;$(xdais_PATH)/packages;$(uia_PATH)/packages;
export XDCPATH

RPE_USEAACDEC  = 1
RPE_USEAACENC  = 1
RPE_USEMP3DEC  = 1
RPE_USETESTCODEC  = 0

XDCROOT = $(xdc_PATH)
XDCTOOLS = $(xdc_PATH)
export XDCROOT
export XDCTOOLS

TMS470CGTOOLPATH = $(CODEGEN_PATH_M3)
CGTOOLS = $(CODEGEN_PATH_DSP)
CGTOOLS_ELF = $(CODEGEN_PATH_DSPELF)
C674CODEGENTOOL = $(CODEGEN_PATH_DSPELF) 

export TMS470CGTOOLPATH
export CGTOOLS
export CGTOOLS_ELF
export C674CODEGENTOOL

CODESOURCERYCGTOOLS = $(CODEGEN_PATH_A8)
export CODESOURCERYCGTOOLS

PATH += $(PATH_SEPARATOR)$(xdc_PATH)$(PATH_SEPARATOR)$(CODEGEN_PATH_DSPELF)/bin$(PATH_SEPARATOR)$(CODEGEN_PATH_M3)/bin
export PATH

# Nothing beyond this point
