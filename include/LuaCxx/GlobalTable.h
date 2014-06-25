#pragma once

#include "LuaCxx/Ref.h"
#include "LuaCxx/State.h"
#include <lua.hpp>

namespace LuaCxx {

struct GlobalTable : public Ref {
protected:
	static State* pushGlobalTable(State* state);

public:
	GlobalTable(State* state);
};

};

