#pragma once

#include "LuaCxx/toC.h"
#include "LuaCxx/fromC.h"
#include "LuaCxx/State.h"
#include "LuaCxx/Stack.h"
#include "Common/Meta.h"
#include <lua.hpp>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>

namespace LuaCxx {

struct State;

//ref wrapper for a stack location

/*
Lua Ref
represents a dereference

how to handle reading values?
operator= seems intuitive, but what of bad cast or nil values?  throw exception?
get(T&) is another option.  I'm going to pass for syntactic reasons.
I'll go with operator>>, even though (unlike basic_stream) these won't be chained.
 however if I was representing the lua stack as a class, then chaining would make sense ...
maybe later I'll change "int index" to "StackRange range" and allow multiple subsequent operator>>'s for multiple return values
*/
struct Ref {
	struct Details {
		State* state;
		int ref;
		bool good;	//handling this like basic_stream
	
		//assumes value to rememer is on top
		//pops value from stack
		Details(State *state_);
		virtual ~Details();
	};

	std::shared_ptr<Details> details;
	
	Ref(State* state);
	Ref(const Ref& value);

	//whether the last IO routine was a success
	bool good() const;

	//type testing
	virtual bool isNil();
	virtual bool isFunction();

	//When using the templaed method the compiler gets confused with const char's
	//so hide it behind explicitly prototyped operator[]'s -- to allow the compiler to coerce types correctly
	//This also serves for explicit getting of a particular type.  Useful for bool, which the compiler doesn't like to deduce correctly.
	template<typename T>
	Ref get(const T& key) {
		lua_State* L = details->state->getState();
		lua_rawgeti(L, LUA_REGISTRYINDEX, details->ref);	//t
		int t = lua_gettop(L);
		fromC<T>(L, key);	//t k
		lua_gettable(L, t);	//t v
		lua_remove(L, t);	//v
		return Ref(details->state);
	}

	//dereference
	virtual Ref operator[](int key) { return get<int>(key); }
	virtual Ref operator[](float key) { return get<float>(key); }
	virtual Ref operator[](double key) { return get<double>(key); }
	virtual Ref operator[](const std::string& key) { return get<std::string>(key); }
	//Speaking of type coercion, defining operator[](bool) also gets coerced from char* before std::string does
	// so defining this causes lua to try to handle the key as a bool rather than a string 
	//virtual Ref operator[](bool key) { return get<bool>(key); }

	//call
	template<typename... Args>
	Ref operator()(Args... args) {
		typedef TypeVector<Args...> ArgVec;
		enum { numArgs = ArgVec::size };
		
		lua_State* L = details->state->getState();
		
		//push function onto stack
		lua_rawgeti(L, LUA_REGISTRYINDEX, details->ref);

		if (!lua_isfunction(L,-1)) throw Common::Exception() << "tried to call a non-function";

		//push args on stack
		ForLoop<0, ArgVec::size, PushArgs<Args...>::template go>::exec(L, args...);

		//do the call
		//only capture 1 argument off the stack 
		details->state->call(numArgs, 1);

		//return a ref
		return Ref(details->state);
	}	
	
	//right now I only test for nil
	//I should test for valid conversion of types as well
	template<typename T>
	Ref& operator>>(T& result);
};

template<typename T>
Ref& Ref::operator>>(T& result) {
	lua_State* L = details->state->getState();
	lua_rawgeti(L, LUA_REGISTRYINDEX, details->ref);	//v
	int v = lua_gettop(L);
	if (lua_isnil(L, v)) {
		details->good = false;
	} else {
		result = toC<T>(L, v);
		details->good = true;
	}
	lua_pop(L, 1);
	return *this;
}

};

