CONFIG_PATH:=$(dir $(lastword $(MAKEFILE_LIST)))
INCLUDE+=$(CONFIG_PATH)include
DYNAMIC_LIBS+=$(CONFIG_PATH)dist/$(PLATFORM)/$(BUILD)/libConfig.dylib
#DYNAMIC_LIBS+=/usr/local/lib/libluajit-5.1.2.0.3.dylib
DYNAMIC_LIBS+=/opt/local/lib/liblua.dylib
INCLUDE+=/opt/local/include
