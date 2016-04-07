LUACXX_PATH:=$(dir $(lastword $(MAKEFILE_LIST)))

include $(LUACXX_PATH)Config.mk

INCLUDE+=$(LUACXX_PATH)include
DYNAMIC_LIBS+=$(LUACXX_PATH)dist/$(PLATFORM)/$(BUILD)/libLuaCxx$(LIB_SUFFIX)

# Lua 5.3
ifdef LUACXX_USE_LUA_5_3
DYNAMIC_LIBS+=$(HOME)/lib/liblua.so
INCLUDE+=$(HOME)/include
endif

# Lua 5.3 as well ... how do you do GNU Make preprocessor logic operations? like `or`?
ifdef LUACXX_USE_LUA_5_3
DYNAMIC_LIBS+=/usr/local/lib/liblua.so
INCLUDE+=/usr/local/include
endif

# LuaJIT 2.0.3
ifdef LUACXX_USE_LUAJIT_2_0_3
DYNAMIC_LIBS+=/usr/local/lib/libluajit-5.1.2.0.3$(LIB_SUFFIX)
INCLUDE+=/usr/local/include/luajit-2.0
LDFLAGS_osx_app+=-pagezero_size 10000 -image_base 100000000
endif
