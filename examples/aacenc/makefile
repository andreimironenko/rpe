# Makefile for aaclc encoder app

ROOTDIR = ../../
APP_NAME = aacenc

SRCDIR = ../common
INCDIR = . ../common

# List all the external components/interfaces, whose interface header files 
#  need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = ipc xdais syslink \
                              ldrmemcfg aaclcenc rpe lindevkit

# List of all external libraries that need to be linked
EXTLIBS_m3video =
EXTLIBS_m3vpss =
EXTLIBS_a8host = $(syslink_PATH)/packages/ti/syslink/lib/syslink.a_debug \
                 $(linuxutils_PATH)/packages/ti/sdo/linuxutils/cmem/lib/cmem.a470MV \
                 $(lindevkit_PATH)/lib/libasound.so \
                 $(ROOTDIR)/lib/libtirpe.a

# List all the components required by the application
COMP_LIST_a8host = rpe
COMP_LIST_c6xdsp =

# Common source files and CFLAGS across all platforms and cores
SRCS_COMMON =
CFLAGS_LOCAL_COMMON =

SRCS_a8host = aacenc.c sysinit_hlos.c
CFLAGS_LOCAL_a8host =

# Core/SoC/platform specific source files and CFLAGS
# Example: 
#   SRCS_<core/SoC/platform-name> = 
#   CFLAGS_LOCAL_<core/SoC/platform-name> =

# Include common make files
include $(ROOTDIR)/makerules/common.mk

# OBJs and libraries are built by using rule defined in rules_<target>.mk 
#     and need not be explicitly specified here

# Nothing beyond this point
