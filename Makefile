# Assumption: These variables will be set in the environment
# $(APPDIR) 
# $(FIRMWARE) 
# $(PATH)
# $(PLATFORM)
# $(TARGETBIN)
# Optionally set $(MAKEOPTS) for things like MODULAR=n DEBUG_BUILD=y USE_SWD_JTAG=y
#$(info APPDIR is $(APPDIR)) 
#$(info FIRMWARE is $(FIRMWARE)) 
#$(info PLATFORM is $(PLATFORM)) 
#$(info TARGETBIN is $(TARGETBIN)) 
#$(info MAKEOPTS is $(MAKEOPTS)) 


all : $(TARGETBIN)

# Use the wildcard function explicitly, because just using a dependency on .h files will
# fail if the project doesn't contain any .h files. This will work either way.
#source := $(wildcard *.cpp) $(wildcard *.h)
source := $(shell find $(SOURCEDIR) -name '*.cpp') $(shell find $(SOURCEDIR) -name '*.h') 

$(TARGETBIN) : $(source)
	cd "$(FIRMWARE)/main" && make all PLATFORM=$(PLATFORM) APPDIR="$(APPDIR)" $(MAKEOPTS)

clean :
	cd "$(FIRMWARE)/modules" && make clean all PLATFORM=$(PLATFORM) APPDIR="$(APPDIR)" $(MAKEOPTS)


.PHONY: all clean