#pragma once

#include "LuaCxx/readFromLuaState.h"
#include "LuaCxx/pushToLuaState.h"
#include "LuaCxx/State.h"
#include "LuaCxx/Stack.h"
#include "Common/Meta.h"
#include <lua.hpp>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>

namespace LuaCxx {

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
	//iterators, for lua_next() iteration
	struct iterator;
	
	struct Details {
		State* state;
		int ref;
		bool good;	//handling this like basic_stream
	
		//assumes value to rememer is on top
		//pops value from stack
		Details(State *state_);
		virtual ~Details();
	
	protected:
		void push();	//push this onto the stack
		friend struct Ref;
		friend struct Ref::iterator;
	};

	std::shared_ptr<Details> details;
	
	Ref(State* state);
	Ref(const Ref& value);

	//whether the last IO routine was a success
	bool good() const;

	//type testing
	virtual bool isBoolean();
	virtual bool isCFunction();
	virtual bool isFunction();
	virtual bool isLightUserData();
	virtual bool isNil();
	virtual bool isNone();
	virtual bool isNoneOrNil();
	virtual bool isNumber();
	virtual bool isString();
	virtual bool isTable();
	virtual bool isThread();
	virtual bool isUserData();

	//When using the templated method the compiler gets confused with const char's
	//so hide it behind explicitly prototyped operator[]'s -- to allow the compiler to coerce types correctly
	//This also serves for explicit getting of a particular type.  Useful for bool, which the compiler doesn't like to deduce correctly.
	template<typename T>
	Ref get(T key) {
		details->push();
		lua_State* L = details->state->getState();
		int t = lua_gettop(L);
		pushToLuaState<T>(L, key);	//t k
		lua_gettable(L, t);	//t v
		lua_remove(L, t);	//v
		return Ref(details->state);
	}

	//dereference
	virtual Ref operator[](int key) { return get<int>(key); }
	virtual Ref operator[](float key) { return get<float>(key); }
	virtual Ref operator[](double key) { return get<double>(key); }
	virtual Ref operator[](const char* key) { return get<const char*>(key); }
	virtual Ref operator[](const std::string& key) { return get<std::string>(key); }
	//Speaking of type coercion, defining operator[](bool) also gets coerced from char* before std::string does
	// so defining this causes lua to try to handle the key as a bool rather than a string 
	//virtual Ref operator[](bool key) { return get<bool>(key); }

	//call
	template<typename... Args>
	Ref operator()(Args... args) {
		typedef TypeVector<Args...> ArgVec;
		enum { numArgs = ArgVec::size };
		
		details->push();
		
		lua_State* L = details->state->getState();
		if (!lua_isfunction(L,-1)) throw Common::Exception() << "tried to call a non-function";

		//push args on stack
		ForLoop<0, ArgVec::size, PushArgs<Args...>::template go>::exec(L, args...);

		//do the call
		//only capture 1 argument off the stack 
		details->state->call(numArgs, 1);

		//return a ref
		return Ref(details->state);
	}	

	//length
	int len();

	//right now I only test for nil
	//I should test for valid conversion of types as well
	//same as with cast, only overload for specific instances
	template<typename T>
	Ref& store(T& result);

	Ref& operator>>(bool& result) { return store<bool>(result); }
	Ref& operator>>(int& result) { return store<int>(result); }
	Ref& operator>>(float& result) { return store<float>(result); }
	Ref& operator>>(double& result) { return store<double>(result); }
	Ref& operator>>(std::string& result) { return store<std::string>(result); }

	bool operator==(const Ref& other) const;
	bool operator!=(const Ref& other) const;

	//Cast operators, for the daring, who want to assign directly without testing .good()
	//I don't want to return a default value on fail, so I'll just have it throw exceptions.
	template<typename T>
	T cast() {
		T result = T();
		store<T>(result);
		if (!good()) throw Common::Exception() << "failed to convert to " << typeid(T).name();
		return result;
	}

	//don't template the cast operator
	//instead only provide what specific cast instances are available
	//these are 1-1 with the readFromLuaState<T> specific instances
	//why not?  because too many cast operator options confuse the compiler when casting to std::string
	operator bool() { return cast<bool>(); }
	operator int() { return cast<int>(); }
	operator float() { return cast<float>(); }
	operator double() { return cast<double>(); }
	operator std::string() { return cast<std::string>(); }
	
	iterator begin();

	/*
	the 'end' iterator is using -1 as the table key as to not equate it with any lua_gettop() legitimate tables
	don't forget that - despite representing an invalid table - this is still a valid lua pseudo-index
	
	TODO get Ref::operator== working, then at least have the table match up, so for a!=b, a.end()!=b.end()
	*/
	iterator end();

	//returns the lua type integer LUA_T*
	int typevalue() const;
	std::string type() const;
};

struct Ref::iterator {
	//need null ctors for end() iterator... 
	State* state;
	Ref value, key, table;	// must be in this order, for ctors to operate in this order, for stack popping / ref generation to work in this order
	bool done;

	iterator(State* state_, bool done_);
	bool operator==(const iterator& other);
	bool operator!=(const iterator& other);
	iterator& operator++();
};

template<typename T>
Ref& Ref::store(T& result) {
	details->push();	//v
	lua_State* L = details->state->getState();
	int v = lua_gettop(L);
	if (lua_isnil(L, v)) {
		details->good = false;
	} else {
		result = readFromLuaState<T>(L, v);
		details->good = true;
	}
	lua_pop(L, 1);
	return *this;
}

}
