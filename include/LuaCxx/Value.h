#pragma once

#include "LuaCxx/toC.h"
#include <lua.hpp>
#include <memory>
#include <string>

namespace LuaCxx {

/*
how to handle reading values?
operator= seems intuitive, but what of bad cast or nil values?  throw exception?
get(T&) is another option.  I'm going to pass for syntactic reasons.
I'll go with operator>>, even though (unlike basic_stream) these won't be chained.
 however if I was representing the lua stack as a class, then chaining would make sense ...
maybe later I'll change "int index" to "StackRange range" and allow multiple subsequent operator>>'s for multiple return values
*/
struct Value {
	struct Details {
		lua_State *L;
		int ref;
		bool good;	//handling this like basic_stream
	
		//assumes value to rememer is on top
		//pops value from stack
		Details(lua_State *L_);
		virtual ~Details();
	};

	std::shared_ptr<Details> details;
	
	Value(lua_State *L);
	Value(const Value &value);
	
	virtual Value operator[](const std::string &key);
	virtual Value operator[](int key);

	template<typename T>
	Value& operator>>(T &result) {
		lua_rawgeti(details->L, LUA_REGISTRYINDEX, details->ref);	//v
		int v = lua_gettop(details->L);
		if (lua_isnil(details->L, v)) {
			details->good = false;
		} else {
			result = toC<T>(details->L, v);
			details->good = true;
		}
		lua_pop(details->L, 1);
		return *this;
	}
};

};

