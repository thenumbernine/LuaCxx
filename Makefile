DIST_FILENAME=LuaCxx
DIST_TYPE=lib

include ../Common/Base.mk
include ../Tensor/Include.mk
include Config.mk

# Lua 5.3
ifdef LUACXX_USE_LUA_5_3
INCLUDE+=$(HOME)/include
endif

# LuaJIT 2.0.3
ifdef LUACXX_USE_LUAJIT_2_0_4
INCLUDE+=/usr/local/include/luajit-2.0
endif
