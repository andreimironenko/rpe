MAJOR := 1
MINOR := 00
PATCH := 00
BUILD := 12

DESTDIR = /
prefix = $(DESTDIR)/usr
bindir = $(prefix)/bin
libdir = $(prefix)/lib
execdir = $(prefix)/share/ti/rpe
includedir = $(prefix)/include

ifdef VERBOSE
Q      :=
SILENT :=
else
Q      := @
SILENT := -s
endif

export Q

PLATFORM ?= ti814x-evm
CORE ?= a8host
ROOTDIR ?= $(PWD)
export ROOTDIR

define makesubdir
	@echo 
	@echo MAKE $1 CORE=$2 PLATFORM=$3
	$(Q)$(MAKE) $(SILENT) -C $1 CORE=$2 PLATFORM=$3
endef

A8_LIB_OBJS = $(shell find lib/obj -name '*.ov5T' -type f)

all: examples firmware 

linuxlib:
	$(call makesubdir,src,a8host,$(PLATFORM))

linuxsharedlib: linuxlib
	@echo
	@echo LIB libtirpe.a libtirpe.so.$(MAJOR).$(MINOR)
	$(Q)$(AR) $(ARFLAGS) lib/libtirpe.a $(A8_LIB_OBJS)
	$(Q)$(CC) -shared -Wl,-soname,libtirpe.so.$(MAJOR).$(MINOR) \
		-o lib/libtirpe.so.$(MAJOR).$(MINOR) $(A8_LIB_OBJS)
	$(Q)ln -sf libtirpe.so.$(MAJOR).$(MINOR) lib/libtirpe.so.$(MAJOR)
	$(Q)ln -sf libtirpe.so.$(MAJOR).$(MINOR) lib/libtirpe.so

dsplib:
	$(call makesubdir,src,c6xdsp,$(PLATFORM))

include makerules/env.mk
include makerules/rules_a8.mk

examples: linuxsharedlib
ifeq ($(RPE_USEAACDEC), 1)
	$(Q)$(MAKE) $(SILENT) -C examples/aacdec CORE=a8host PLATFORM=$(PLATFORM)
endif
ifeq ($(RPE_USEAACENC), 1)
	$(Q)$(MAKE) $(SILENT) -C examples/aacenc CORE=a8host PLATFORM=$(PLATFORM)
endif
ifeq ($(RPE_USEMP3DEC), 1)
	$(Q)$(MAKE) $(SILENT) -C examples/mp3dec CORE=a8host PLATFORM=$(PLATFORM)
endif
ifeq ($(RPE_USETESTCODEC), 1)
	$(Q)$(MAKE) $(SILENT) -C tests/testcases/test1 CORE=a8host PLATFORM=$(PLATFORM)
	$(Q)$(MAKE) $(SILENT) -C tests/testcases/test2 CORE=a8host PLATFORM=$(PLATFORM)
endif	

firmware: dsplib rpetests
	$(Q)$(MAKE) $(SILENT) -C examples -f xdcdepmakefile _APP=dm81xx CORE=c6xdsp PLATFORM=$(PLATFORM)
	$(Q)$(MAKE) $(SILENT) -C examples/dm81xx xdc_configuro CORE=c6xdsp PLATFORM=$(PLATFORM)
	$(Q)$(MAKE) $(SILENT) -C examples/dm81xx CORE=c6xdsp PLATFORM=$(PLATFORM)

rpetests:
ifeq ($(RPE_USETESTCODEC), 1)
	$(Q)$(MAKE) $(SILENT) -C tests/copycodec CORE=c6xdsp PLATFORM=$(PLATFORM)
endif	
       
install:
	$(Q)install -d $(libdir) $(includedir)/ti $(bindir) $(execdir)
	$(Q)install lib/libtirpe.a $(libdir)
	$(Q)install lib/libtirpe.so.$(MAJOR).$(MINOR) $(libdir)
	$(Q)ln -sf libtirpe.so.$(MAJOR).$(MINOR) $(libdir)/libtirpe.so.$(MAJOR)
	$(Q)ln -sf libtirpe.so.$(MAJOR).$(MINOR) $(libdir)/libtirpe.so
	$(Q)install include/ti/* $(includedir)/ti
	$(Q)install lib/dm81xx/bin/$(PLATFORM)/* $(execdir)
ifeq ($(RPE_USEAACDEC), 1)
	$(Q)install lib/aacdec/bin/$(PLATFORM)/* $(execdir)
	$(Q)install examples/aacdec/*.cfg $(execdir)
	$(Q)install examples/aacdec/*.aac $(execdir)
endif
ifeq ($(RPE_USEAACENC), 1)
	$(Q)install lib/aacenc/bin/$(PLATFORM)/* $(execdir)
	$(Q)install examples/aacenc/*.cfg $(execdir)
	$(Q)install examples/aacenc/*.wav $(execdir)
endif
ifeq ($(RPE_USEMP3DEC), 1)
	$(Q)install lib/mp3dec/bin/$(PLATFORM)/* $(execdir)
	$(Q)install examples/mp3dec/*.cfg $(execdir)
	$(Q)install examples/mp3dec/*.mp3 $(execdir)
endif
ifeq ($(RPE_USETESTCODEC), 1)
	$(Q)install lib/test1/bin/$(PLATFORM)/* $(execdir)
	$(Q)install lib/test2/bin/$(PLATFORM)/* $(execdir)
endif

clean:
	@echo "CLEAN lib bin"
	$(Q)$(MAKE) $(SILENT) -C examples -f xdcdepmakefile xdcdepclean _APP=dm81xx CORE=c6xdsp	
	$(Q)$(RM) -fr lib bin

help:
	@echo
	@echo "Available build targets are  :"
	@echo
	@echo "    all                            : Build RPE Framework and DSP Firmware"
	@echo "    linuxsharedlib                 : Build Linux libraries"
	@echo "    dsplib                         : Build DSP libraries"
	@echo "    firmware                       : Build DSP Firmware"
	@echo "    examples                       : Build Example Applications"
	@echo "    clean                          : Remove files generated"
	@echo "    install                        : Install libraries, header files and DSP binary"
	@echo
	@echo "To build you need to override variables set in env.mk"

.show-products:
	@echo "bios         - $(bios_PATH)"
	@echo "xdc          - $(xdc_PATH)"
	@echo "ipc          - $(ipc_PATH)"
	@echo "syslink      - $(syslink_PATH)"
	@echo "xdais        - $(xdais_PATH)"
	@echo "mcutils      - $(mcutils_PATH)"
	@echo "linuxutils   - $(linuxutils_PATH)"
	@echo "uia          - $(uia_PATH)"
	@echo "CodeSourcery - $(CODESOURCERY_PATH)"
	@echo "C6x CGTOOLS  - $(CODEGEN_PATH_DSPELF)"
	@echo 
	@echo "aaclcdec     - $(aaclcdec_PATH)"
	@echo "aaclcenc     - $(aaclcenc_PATH)"
	@echo "mp3dec       - $(mp3dec_PATH)"
	@echo 
	@echo "RPE version $(MAJOR).$(MINOR).$(PATCH).$(BUILD) $(RELTYPE)"

# Nothing beyond this point
