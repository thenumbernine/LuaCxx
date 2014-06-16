#include "LuaCxx/Value.h"

namespace LuaCxx {

//assumes value to rememer is on top
//pops value from stack
Value::Details::Details(lua_State *L_)
: L(L_)
, ref(luaL_ref(L, LUA_REGISTRYINDEX))
, good(true)
{}

Value::Details::~Details() {
	luaL_unref(L, LUA_REGISTRYINDEX, ref);
}


Value::Value(lua_State *L) : details(std::make_shared<Details>(L)) {}
Value::Value(const Value &value) : details(value.details) {}
	
bool Value::good() const { return details->good; }

Value Value::operator[](const std::string &key) {
	lua_rawgeti(details->L, LUA_REGISTRYINDEX, details->ref);	//t
	int t = lua_gettop(details->L);
	lua_pushlstring(details->L, key.c_str(), key.length());	//t k
	lua_gettable(details->L, t);	//t v
	lua_remove(details->L, t);	//v
	return Value(details->L);
}

Value Value::operator[](int key) {
	lua_rawgeti(details->L, LUA_REGISTRYINDEX, details->ref);	//t
	int t = lua_gettop(details->L);
	lua_pushinteger(details->L, key);	//t k
	lua_gettable(details->L, t);	//t v
	lua_remove(details->L, t);	//v
	return Value(details->L);
}

};

