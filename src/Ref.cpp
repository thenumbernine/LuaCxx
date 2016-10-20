#include "LuaCxx/Ref.h"
#include <lua.hpp>

namespace LuaCxx {

void Ref::Details::push() {
	lua_rawgeti(state->getState(), LUA_REGISTRYINDEX, ref);
}

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
int testType(lua_State* L) {
	bool is = !!testFunc(L, -1);
	lua_pop(L, 1);
	return is;
}

bool Ref::isBoolean() { details->push(); return testType<_lua_isboolean>(details->state->getState()); }
bool Ref::isCFunction() { details->push(); return testType<lua_iscfunction>(details->state->getState()); }
bool Ref::isFunction() { details->push(); return testType<_lua_isfunction>(details->state->getState()); }
bool Ref::isLightUserData() { details->push(); return testType<_lua_islightuserdata>(details->state->getState()); }
bool Ref::isNil() { details->push(); return testType<_lua_isnil>(details->state->getState()); }
bool Ref::isNone() { details->push(); return testType<_lua_isnone>(details->state->getState()); }
bool Ref::isNoneOrNil() { details->push(); return testType<_lua_isnoneornil>(details->state->getState()); }
bool Ref::isNumber() { details->push(); return testType<lua_isnumber>(details->state->getState()); }
bool Ref::isString() { details->push(); return testType<lua_isstring>(details->state->getState()); }
bool Ref::isTable() { details->push(); return testType<_lua_istable>(details->state->getState()); }
bool Ref::isThread() { details->push(); return testType<_lua_isthread>(details->state->getState()); }
bool Ref::isUserData() { details->push(); return testType<lua_isuserdata>(details->state->getState()); }

int Ref::len() {
	details->push();
	lua_State* L = details->state->getState();
	lua_len(L, -1);
	int len = toC<int>(L, -1);
	lua_pop(L, 2);
	return len;
}

bool Ref::operator==(const Ref& other) const {
	lua_State* L = details->state->getState();
	int top = lua_gettop(L);
	details->push();		//a
	other.details->push();	//a b
#if LUA_VERSION_NUM == 501
	bool result = !!lua_equal(L, top+1, top+2);
#else
	bool result = !!lua_compare(L, top+1, top+2, LUA_OPEQ);
#endif
	lua_settop(L, top);
	return result;
}

bool Ref::operator!=(const Ref& other) const {
	return !operator==(other);
}

Ref::iterator::iterator(State* state_, bool done_)
: state(state_)
, value(state_)	//first pop the value from the top
, key(state_)	//next pop the key from the top
, table(state_)	//next pop the table
, done(done_)
{}

bool Ref::iterator::operator==(const iterator& other) {
	if (table != other.table) return false;
	if (key != other.key) return false;
	if (done != other.done) return true;
	return true;
}

bool Ref::iterator::operator!=(const iterator& other) {
	return !operator==(other);
}

Ref::iterator& Ref::iterator::operator++() {
	lua_State* L = state->getState();
	table.details->push();	//t
	int t = lua_gettop(L);
	key.details->push();	//t key
	done = !lua_next(L, t);	//t key value
	value = Ref(state);		//t key
	key = Ref(state);		//t
	lua_pop(L,1);
	return *this;
}

Ref::iterator Ref::begin() {
	details->push();
	lua_State* L = details->state->getState();
	int t = lua_gettop(L);
	lua_pushnil(L);				//t nil
	bool done = !lua_next(L, t);//t key value
	return iterator(details->state, done);
}

Ref::iterator Ref::end()  {
	lua_State* L = details->state->getState(); 
	lua_pushnil(L);
	lua_pushnil(L);
	lua_pushnil(L);
	return iterator(details->state, true);
}

};
