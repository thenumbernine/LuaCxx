CONFIG_PATH:=$(dir $(lastword $(MAKEFILE_LIST)))
INCLUDE+=$(CONFIG_PATH)include
DYNAMIC_LIBS+=$(CONFIG_PATH)dist/$(PLATFORM)/$(BUILD)/libImage.dylib
DYNAMIC_LIBS+=/usr/local/lib/libluajit-5.1.2.0.3.dylib
