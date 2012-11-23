# Filename: platform.mk
#
# Platforms make file - Platform/SoC/targets are defined/derived in this file
# 
# This file needs to change when:
#     1. a new platform/SoC is added, which also might have its own cores/ISAs
#
# This file does not require to be changed unless you are porting to a new SoC.

#
# Derive SOC from PLATFORM
#

# ti816x (Netra) catalog EVM
ifeq ($(PLATFORM),ti816x-evm)
 SOC = ti816x
 ifeq ($(CORE),c6xdsp)
  PLATFORM_XDC = "ti.platforms.evmDM8168"
 endif 
endif

# ti814x (Centaurus) Catalog EVM
ifeq ($(PLATFORM),ti814x-evm)
 SOC = ti814x
 ifeq ($(CORE),c6xdsp)
   PLATFORM_XDC = "ti.platforms.evmDM8148"
 endif 
endif

# ti811x (J5ECO) Catalog EVM
ifeq ($(PLATFORM),ti811x-evm)
 SOC = ti811x
 ifeq ($(CORE),c6xdsp)
   PLATFORM_XDC = "ti.platforms.evmTI811X"
 endif 
endif

# Derive Target/ISA from CORE

# m3vpss
ifeq ($(CORE),m3vpss)
 ISA = m3
endif

# m3video
ifeq ($(CORE),m3video)
 ISA = m3
endif

# a8host
ifeq ($(CORE),a8host)
 ISA = a8
endif

# c6xdsp
ifeq ($(CORE),c6xdsp)
 ISA = c674
endif

#
# Derive XDC/ISA specific settings 
#

ifeq ($(ISA),m3)
  TARGET_XDC = ti.targets.arm.elf.M3
  FORMAT_EXT = e

  ifeq ($(CORE),m3video)
    PLATFORM_XDC = "ti.platforms.generic:DucatiPlatform_Core0"
  else
    PLATFORM_XDC = "ti.platforms.generic:DucatiPlatform_Core1"
  endif

  # Define the file extensions
  OBJEXT = oem3
  LIBEXT = aem3
  EXEEXT = xem3
  ASMEXT = sem3
endif

ifeq ($(ISA),c674)
  TARGET_XDC = ti.targets.elf.C674
  FORMAT_EXT = e

  # Define the file extensions
  OBJEXT = oe674
  LIBEXT = ae674
  EXEEXT = xe674
  ASMEXT = se674
endif

ifeq ($(ISA),a8)
  # No XDC definitions are required for A8 Linux

  # Define the file extensions
  OBJEXT = ov5T
  LIBEXT = av5T
  EXEEXT = xv5T
  ASMEXT = sv5T
  DLLEXT = so
endif

export PLATFORM_MEM
export SOC

ifeq ($(CORE),m3vpss)
  CFGARGS_XDC = \"{mode: \\\"$(IPC_MODE)\\\", coreName: \\\"VPSS-M3\\\", tiler: \\\"DucatiTilerMemMgr\\\", platFormMem: \\\"$(PLATFORM_MEM)\\\", maxResolution: \\\"$(MAX_RESOLUTION)\\\", BuildPlatform: \\\"$(OMX_PLATFORM)\\\"}\"
endif
ifeq ($(CORE),m3video)
  CFGARGS_XDC = \"{mode: \\\"$(IPC_MODE)\\\", coreName: \\\"VIDEO-M3\\\", tiler: \\\"DucatiTilerMemMgr\\\", platFormMem: \\\"$(PLATFORM_MEM)\\\", maxResolution: \\\"$(MAX_RESOLUTION)\\\", BuildPlatform: \\\"$(OMX_PLATFORM)\\\"}\"
endif
ifeq ($(CORE),c6xdsp)
  CFGARGS_XDC = \"{mode: \\\"$(IPC_MODE)\\\", coreName:\\\"DSP\\\", tiler: \\\"NullTilerMemMgr\\\", platFormMem: \\\"$(PLATFORM_MEM)\\\", maxResolution: \\\"$(MAX_RESOLUTION)\\\", BuildPlatform: \\\"$(PLATFORM)\\\"}\" 
endif
ifeq ($(CORE),a8host)
  CFGARGS_XDC = \"{mode: \\\"$(IPC_MODE)\\\", coreName:\\\"HOST\\\", tiler: \\\"NullTilerMemMgr\\\", platFormMem: \\\"$(PLATFORM_MEM)\\\", maxResolution: \\\"$(MAX_RESOLUTION)\\\", BuildPlatform: \\\"$(OMX_PLATFORM)\\\"}\"
endif

# Nothing beyond this point
