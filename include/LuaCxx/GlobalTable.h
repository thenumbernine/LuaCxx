#pragma once

#include "LuaCxx/Value.h"
#include "LuaCxx/State.h"
#include <lua.hpp>

namespace LuaCxx {

struct GlobalTable : public Value {
protected:
	static State* pushGlobalTable(State* state);

public:
	GlobalTable(State* state);
};

};

