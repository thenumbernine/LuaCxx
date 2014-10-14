DIST_FILENAME=LuaCxx
DIST_TYPE=lib
include ../Common/Base.mk
include ../Tensor/Include.mk

# Lua 5.2
INCLUDE+=/opt/local/include

# LuaJIT 2.0.3
#INCLUDE+=/usr/local/include/luajit-2.0
