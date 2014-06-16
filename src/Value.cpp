#include "LuaCxx/Value.h"

namespace LuaCxx {

//assumes value to rememer is on top
//pops value from stack
Value::Details::Details(State* state_)
: state(state_)
, ref(luaL_ref(state->getState(), LUA_REGISTRYINDEX))
, good(true)
{}

Value::Details::~Details() {
	luaL_unref(state->getState(), LUA_REGISTRYINDEX, ref);
}


Value::Value(State* state) : details(std::make_shared<Details>(state)) {}
Value::Value(const Value &value) : details(value.details) {}
	
bool Value::good() const { return details->good; }

bool Value::isNil() {
	lua_State* L = details->state->getState();
	lua_rawgeti(L, LUA_REGISTRYINDEX, details->ref);
	bool isNil = lua_isnil(L, -1);
	lua_pop(L, 1);
	return isNil;
}

bool Value::isFunction() {
	lua_State* L = details->state->getState();
	lua_rawgeti(L, LUA_REGISTRYINDEX, details->ref);
	bool isFunction = lua_isfunction(L, -1);
	lua_pop(L, 1);
	return isFunction;
}

Value Value::operator[](const std::string &key) {
	lua_State* L = details->state->getState();
	lua_rawgeti(L, LUA_REGISTRYINDEX, details->ref);	//t
	int t = lua_gettop(L);
	lua_pushlstring(L, key.c_str(), key.length());	//t k
	lua_gettable(L, t);	//t v
	lua_remove(L, t);	//v
	return Value(details->state);
}

Value Value::operator[](int key) {
	lua_State* L = details->state->getState();
	lua_rawgeti(L, LUA_REGISTRYINDEX, details->ref);	//t
	int t = lua_gettop(L);
	lua_pushinteger(L, key);	//t k
	lua_gettable(L, t);	//t v
	lua_remove(L, t);	//v
	return Value(details->state);
}

};

