#include "LuaCxx/GlobalTable.h"

namespace LuaCxx {

State * GlobalTable::pushGlobalTable(State * state) {
	lua_State * L = state->getState();
#if LUA_VERSION_NUM == 501
	lua_pushvalue(L, LUA_GLOBALSINDEX);
#elif LUA_VERSION_NUM >= 502
	lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
#else
#error unknown Lua version
#endif
	return state;
}

//push the global table onto the stack before constructing the value object
GlobalTable::GlobalTable(State * state) 
: Ref(pushGlobalTable(state)) 
{}

};

