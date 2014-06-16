#pragma once

#include "LuaCxx/toC.h"
#include "LuaCxx/fromC.h"
#include "LuaCxx/State.h"
#include <lua.hpp>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>

namespace LuaCxx {

struct State;

//ref wrapper for a stack location

//used for pushing args on stack
template<typename Ret, typename... Args>
struct ConvertArg {
	typedef Function<Ret(Args...)> F;
	template<int index>
	struct go {
		static bool exec(lua_State *L, Args... args) {
			typedef typename F::template Arg<index> ArgI;
			std::tuple<Args...> t = std::make_tuple(args...);
			ArgI arg = std::get<index>(t);
			fromC<ArgI>(L, arg);
			return false;
		}
	};
};

/*
Lua Value
represents a dereference
TODO call this 'Ref'

how to handle reading values?
operator= seems intuitive, but what of bad cast or nil values?  throw exception?
get(T&) is another option.  I'm going to pass for syntactic reasons.
I'll go with operator>>, even though (unlike basic_stream) these won't be chained.
 however if I was representing the lua stack as a class, then chaining would make sense ...
maybe later I'll change "int index" to "StackRange range" and allow multiple subsequent operator>>'s for multiple return values
*/
struct Value {
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
	
	Value(State* state);
	Value(const Value& value);

	//whether the last IO routine was a success
	bool good() const;

	//type testing
	virtual bool isNil();
	virtual bool isFunction();
	
	//dereference
	virtual Value operator[](const std::string& key);
	virtual Value operator[](int key);

	//call
	template<typename Ret, typename... Args>
	Ret call(Args... args) {
		typedef Function<Ret(Args...)> F;
		enum { numArgs = F::numArgs };
		int numRet = std::is_same<Ret, void>::value ? 0 : 1;
		
		lua_State* L = details->state->getState();
		
		//push function onto stack
		lua_rawgeti(L, LUA_REGISTRYINDEX, details->ref);

		if (!lua_isfunction(L,-1)) throw Common::Exception() << "tried to call a non-function";

		//push args on stack
		ForLoop<0, F::numArgs, ConvertArg<Ret, Args...>::template go>::exec(L, args...);

		//do the call
		details->state->call(numArgs, numRet);

		struct DoNothing {
			static void go(lua_State*) {}
		};

		struct ConvertStackTop {
			static Ret go(lua_State* L) {
				Ret result = toC<Ret>(L, lua_gettop(L));
				lua_pop(L,1);
				return result;
			}
		};

		return If<
			std::is_same<Ret, void>::value,
			DoNothing,
			ConvertStackTop>::Type::go(L);
	}	
	
	//right now I only test for nil
	//I should test for valid conversion of types as well
	template<typename T>
	Value& operator>>(T& result);
};

template<typename T>
Value& Value::operator>>(T& result) {
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

