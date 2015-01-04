LUACXX_PATH:=$(dir $(lastword $(MAKEFILE_LIST)))

include $(LUACXX_PATH)Config.mk

INCLUDE+=$(LUACXX_PATH)include
DYNAMIC_LIBS+=$(LUACXX_PATH)dist/$(PLATFORM)/$(BUILD)/libLuaCxx.dylib

# Lua 5.2
ifdef LUACXX_USE_LUA_5_2
DYNAMIC_LIBS+=/opt/local/lib/liblua.dylib
INCLUDE+=/opt/local/include
endif

# LuaJIT 2.0.3
ifdef LUACXX_USE_LUAJIT_2_0_3
DYNAMIC_LIBS+=/usr/local/lib/libluajit-5.1.2.0.3.dylib
INCLUDE+=/usr/local/include/luajit-2.0
LDFLAGS_osx+=-pagezero_size 10000 -image_base 100000000
endif

