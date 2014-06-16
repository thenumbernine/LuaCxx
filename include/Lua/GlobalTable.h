#pragma once

#include "Lua/Value.h"
#include <lua.hpp>

namespace Lua {

struct GlobalTable : public Value {
protected:
	static lua_State *pushGlobalTable(lua_State *L);

public:
	GlobalTable(lua_State *L);
};

};

