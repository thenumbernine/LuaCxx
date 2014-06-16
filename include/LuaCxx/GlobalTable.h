#pragma once

#include "LuaCxx/Value.h"
#include <lua.hpp>

namespace LuaCxx {

struct GlobalTable : public Value {
protected:
	static lua_State *pushGlobalTable(lua_State *L);

public:
	GlobalTable(lua_State *L);
};

};

