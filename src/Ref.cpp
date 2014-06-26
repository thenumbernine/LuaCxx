#include "LuaCxx/Ref.h"

namespace LuaCxx {

//assumes value to rememer is on top
//pops value from stack
Ref::Details::Details(State* state_)
: state(state_)
, ref(luaL_ref(state->getState(), LUA_REGISTRYINDEX))
, good(true)
{}

Ref::Details::~Details() {
	luaL_unref(state->getState(), LUA_REGISTRYINDEX, ref);
}


Ref::Ref(State* state) : details(std::make_shared<Details>(state)) {}
Ref::Ref(const Ref &value) : details(value.details) {}
	
bool Ref::good() const { return details->good; }

bool Ref::isNil() {
	lua_State* L = details->state->getState();
	lua_rawgeti(L, LUA_REGISTRYINDEX, details->ref);
	bool isNil = lua_isnil(L, -1);
	lua_pop(L, 1);
	return isNil;
}

bool Ref::isFunction() {
	lua_State* L = details->state->getState();
	lua_rawgeti(L, LUA_REGISTRYINDEX, details->ref);
	bool isFunction = lua_isfunction(L, -1);
	lua_pop(L, 1);
	return isFunction;
}

};

