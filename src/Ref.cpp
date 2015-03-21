#include "LuaCxx/Ref.h"
#include <lua.hpp>

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

#define FUNCTION_FOR_MACRO(name) static int _##name(lua_State* L, int n) { return name(L, n); }
FUNCTION_FOR_MACRO(lua_isfunction)
FUNCTION_FOR_MACRO(lua_istable)
FUNCTION_FOR_MACRO(lua_islightuserdata)
FUNCTION_FOR_MACRO(lua_isnil)
FUNCTION_FOR_MACRO(lua_isboolean)
FUNCTION_FOR_MACRO(lua_isthread)
FUNCTION_FOR_MACRO(lua_isnone)
FUNCTION_FOR_MACRO(lua_isnoneornil)

template<int (*testFunc)(lua_State*, int)>
int testType(lua_State* L, int refIndex) {
	lua_rawgeti(L, LUA_REGISTRYINDEX, refIndex);
	bool is = !!testFunc(L, -1);
	lua_pop(L, 1);
	return is;
}

bool Ref::isBoolean() { return testType<_lua_isboolean>(details->state->getState(), details->ref); }
bool Ref::isCFunction() { return testType<lua_iscfunction>(details->state->getState(), details->ref); }
bool Ref::isFunction() { return testType<_lua_isfunction>(details->state->getState(), details->ref); }
bool Ref::isLightUserData() { return testType<_lua_islightuserdata>(details->state->getState(), details->ref); }
bool Ref::isNil() { return testType<_lua_isnil>(details->state->getState(), details->ref); }
bool Ref::isNone() { return testType<_lua_isnone>(details->state->getState(), details->ref); }
bool Ref::isNoneOrNil() { return testType<_lua_isnoneornil>(details->state->getState(), details->ref); }
bool Ref::isNumber() { return testType<lua_isnumber>(details->state->getState(), details->ref); }
bool Ref::isString() { return testType<lua_isstring>(details->state->getState(), details->ref); }
bool Ref::isTable() { return testType<_lua_istable>(details->state->getState(), details->ref); }
bool Ref::isThread() { return testType<_lua_isthread>(details->state->getState(), details->ref); }
bool Ref::isUserData() { return testType<lua_isuserdata>(details->state->getState(), details->ref); }

};

