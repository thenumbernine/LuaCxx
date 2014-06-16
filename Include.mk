LUACXX_PATH:=$(dir $(lastword $(MAKEFILE_LIST)))
INCLUDE+=$(LUACXX_PATH)include
DYNAMIC_LIBS+=$(LUACXX_PATH)dist/$(PLATFORM)/$(BUILD)/libLuaC++.dylib

#working
DYNAMIC_LIBS+=/opt/local/lib/liblua.dylib
INCLUDE+=/opt/local/include

#failing to create state...
#DYNAMIC_LIBS+=/usr/local/lib/libluajit-5.1.2.0.3.dylib
#INCLUDE+=/usr/local/include/luajit-2.0
