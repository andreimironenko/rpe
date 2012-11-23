MODULE_NAME = rpe

-include config/codecs.mk
-include ../makerules/env.mk

INCLUDE_EXTERNAL_INTERFACES = bios syslink ipc xdc xdais ldrmemcfg rpe linuxutils $(INCLUDE_CODEC_INTERFACES)

SRCDIR = config rpecore rpexdm utils
INCDIR = include

ifeq ($(RPE_USEAACDEC), 1)
CFLAGS_LOCAL_COMMON += -DRPE_AACDEC_ENABLE
endif
ifeq ($(RPE_USEAACENC), 1)
CFLAGS_LOCAL_COMMON += -DRPE_AACENC_ENABLE
endif
ifeq ($(RPE_USEMP3DEC), 1)
CFLAGS_LOCAL_COMMON += -DRPE_MP3DEC_ENABLE
endif
ifeq ($(RPE_USEJPEGDEC), 1)
CFLAGS_LOCAL_COMMON += -DRPE_JPEGDEC_ENABLE
endif
ifeq ($(RPE_USETESTCODEC), 1)
CFLAGS_LOCAL_COMMON += -DRPE_TESTCODEC_ENABLE
endif

SRCS_COMMON =   	rpe_fxncall.c \
		  	utils_shmem.c \
			ipc_config.c \

SRCS_a8host =	rpe_client.c \
			xdm_client.c \
			utils_error.c \
			client_config.c

SRCS_c6xdsp = 	rpe_server.c \
			xdm_server.c \
			xdm_server_marshall.c \
			utils_task_bios.c \
			server_task_config.c \
			server_config_c67x.c

include ../makerules/common.mk

# Nothing beyond this
