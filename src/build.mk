# Default behaviour. DO NOT CHANGE!
INCLUDE_DIRS += $(SOURCE_PATH)/$(USRSRC)  # add user sources to include path
CPPSRC += $(call target_files,$(USRSRC_SLASH),*.cpp)
CSRC += $(call target_files,$(USRSRC_SLASH),*.c)

APPSOURCES=$(call target_files,$(USRSRC_SLASH),*.cpp)
APPSOURCES+=$(call target_files,$(USRSRC_SLASH),*.c)

# Custom behaviour
INCLUDE_DIRS += $(SOURCE_PATH)/inc