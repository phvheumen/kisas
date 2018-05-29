# Assumption: These variables will be set in the environment
# $(APPDIR) 
# $(FIRMWARE) 
# $(PATH)
# $(PLATFORM)
# $(TARGETBIN)
# Optionally set $(MAKEOPTS) for things like MODULAR=n DEBUG_BUILD=y USE_SWD_JTAG=y

# This part of OS detecting needs some tweaking. Since CYGWIN_NT-10.0 is also detected outside the CYGWIN terminal.
# The reason why we want OS detection is get this makefile as multiplatform as possible

ifdef $(APPDIR)

detected_OS := $(shell sh -c 'uname -s 2>/dev/null || echo NOT')
$(info Detected OS: $(detected_OS))
$(info Path: $(cd))
$(info Path: $(shell echo $(cd)))

ifeq ($(detected_OS), CYGWIN_NT-10.0)
	APPDIR=$(shell cygpath -m "$(PWD)")
else
	APPDIR=$(PWD)
endif

endif # ifdef APPDIR

FIRMWARE=./firmware
PLATFORM=electron
TARGETBIN=target/kisas.bin
MAKEOPTS=MODULAR=n DEBUG_BUILD=y USE_SWD_JTAG=y

$(info APPDIR: $(APPDIR)) 
$(info FIRMWARE: $(FIRMWARE)) 
$(info PLATFORM: $(PLATFORM)) 
$(info TARGETBIN: $(TARGETBIN)) 
$(info MAKEOPTS: $(MAKEOPTS)) 

all : $(TARGETBIN)

headers := $(shell find $(SOURCEDIR) -name '*.h' -not -path './firmware/*') 
sources := $(shell find $(SOURCEDIR) -name '*.cpp' -not -path './firmware/*') 
dependencies := $(headers) $(sources)

#$(info Header files:)
#$(foreach file,$(headers),$(info $(file));)

#$(info Source files:)
#$(foreach file,$(sources),$(info $(file));)

$(TARGETBIN) : $(dependencies)
	cd "$(FIRMWARE)/main" && make all PLATFORM=$(PLATFORM) APPDIR="$(APPDIR)" $(MAKEOPTS)

clean :
	cd "$(FIRMWARE)/modules" && make clean all PLATFORM=$(PLATFORM) APPDIR="$(APPDIR)" $(MAKEOPTS)


.PHONY: all clean