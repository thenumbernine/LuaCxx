#include "LuaCxx/GlobalTable.h"

namespace LuaCxx {

State* GlobalTable::pushGlobalTable(State* state) {
	lua_State* L = state->getState();
	lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
	return state;
}

//push the global table onto the stack before constructing the value object
GlobalTable::GlobalTable(State* state) 
: Value(pushGlobalTable(state)) 
{}

};

