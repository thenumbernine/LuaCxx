#pragma once

#include "LuaCxx/Ref.h"
#include <lua.hpp>

namespace LuaCxx {

struct State;

struct GlobalTable : public Ref {
protected:
	static State* pushGlobalTable(State* state);

public:
	GlobalTable(State* state);
};

};

