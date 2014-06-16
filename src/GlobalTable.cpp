#include "LuaCxx/GlobalTable.h"

namespace LuaCxx {

lua_State* GlobalTable::pushGlobalTable(lua_State* L) {
	lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
	return L;
}

//push the global table onto the stack before constructing the value object
GlobalTable::GlobalTable(lua_State *L) 
: Value(pushGlobalTable(L)) 
{}

};

