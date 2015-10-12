DIST_FILENAME=LuaCxx
DIST_TYPE=lib

include ../Common/Base.mk
include ../Tensor/Include.mk
include Config.mk

# Lua 5.2
ifdef LUACXX_USE_LUA_5_2
INCLUDE+=/usr/local/include
endif

# LuaJIT 2.0.3
ifdef LUACXX_USE_LUAJIT_2_0_3
INCLUDE+=/usr/local/include/luajit-2.0
endif

