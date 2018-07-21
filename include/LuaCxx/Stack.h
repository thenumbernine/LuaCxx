#pragma once

#include "LuaCxx/State.h"
#include "LuaCxx/fromC.h"
#include "LuaCxx/toC.h"
#include <tuple>

namespace LuaCxx {

//used for pushing args on stack
template<typename... Args>
struct PushArgs {
	typedef TypeVector<Args...> ArgVec;
	template<int index>
	struct go {
		static bool exec(lua_State *L, Args... args) {
			typedef typename ArgVec::template Get<index> ArgI;
			std::tuple<Args...> t = std::make_tuple(args...);
			ArgI arg = std::get<index>(t);
			fromC<ArgI>(L, arg);
			return false;
		}
	};
};

//used for popping args on stack
template<typename... Args>
struct PopArgs {
	typedef TypeVector<Args...> ArgVec;
	template<int index>
	struct go {
		static bool exec(lua_State *L, Args&... args) {
			typedef typename ArgVec::template Get<index> ArgI;
			std::tuple<Args&...> t = std::forward_as_tuple(args...);
			std::get<ArgVec::size - index - 1>(t) = toC<typename std::remove_reference<ArgI>::type>(L, -1);
			lua_pop(L, 1);
			return false;
		}
	};
};



/*
Reference for stack operations.
Saves current stack size upon construction.
Restores that size upon destruction.
*/
struct Stack {
protected:
	State* state;
	int topIndex;

	//factory constructor 
	Stack(State* state);
	
	//copy constructor
	Stack(const Stack& other);
	
	friend struct State;
public:
	
	//move constructor
	Stack(Stack&& other);

	virtual ~Stack();

	//push primitive
	template<typename T>
	Stack& operator<<(const T& value) {
		fromC<T>(state->getState(), value);
		return *this;
	}

	//pop primitive 
	template<typename T>
	Stack& operator>>(T& value) {
		lua_State* L = state->getState();
		value = toC<T>(L, -1);
		lua_pop(L, 1);
		return *this;
	}

	//push multiple
	//TODO rename to pushconst, not to confuse with 'lua_pushvalue'
	template<typename... Args>
	Stack& push(Args... args) {
		typedef TypeVector<Args...> ArgVec;
		ForLoop<0, ArgVec::size, PushArgs<Args...>::template go>::exec(state->getState(), args...);
		return *this;
	}

	//pop multiple
	//order lines up with the stack and with multiple return assignment, so pop(a,b,c) gives c the top, then b, then a
	// this does not line up with operator>>, so stack.pop(a,b,c) is equivalent to stack >> c >> b >> a
	template<typename... Args>
	Stack& pop(Args&&... args) {
		ForLoop<0, TypeVector<Args...>::size, PopArgs<Args&&...>::template go>::exec(state->getState(), args...);
		return *this;
	}

	//templated
	template<typename T>
	Stack& getType(const T& key, int tableLoc = -1) {
		lua_State* L = state->getState();
		//why jump through hoops with copying the table? 
		// in case tableLoc is a special/negative number.
		lua_pushvalue(L, tableLoc);	//t
		fromC<T>(L, key);	//t k
		lua_gettable(L, -2);	//t v
		lua_remove(L, -2);	//v
		return *this;
	}

	//get a key from the table location (default at the top of the stack)
	// and push it onto the stack
	//notice, lua format is lua_get**(tableLoc, key) ... but this doesn't let me default tableLoc to -1 
	Stack& get(int key, int tableLoc = -1) { return getType<int>(key, tableLoc); }
	Stack& get(float key, int tableLoc = -1) { return getType<float>(key, tableLoc); }
	Stack& get(double key, int tableLoc = -1) { return getType<double>(key, tableLoc); }
	Stack& get(const std::string& key, int tableLoc = -1) { return getType<std::string>(key, tableLoc); }

	//lua_pushvalue
	Stack& pushvalue(int index = -1);

	//pushes a new table onto the stack
	Stack& newtable();

	//pops top of stack, sets it as a global
	Stack& setGlobal(std::string name);

	//sets the value on the stack to the table's index
	//default the table index to -2, since the value must be at -1
	Stack& seti(lua_Integer key, int tableLoc = -2);

	//get a key from the global table and push it into the stack
	template<typename T>
	Stack& getGlobalType(const T& key) {
#if LUA_VERSION_NUM == 501
		get(key, LUA_GLOBALSINDEX);
#elif LUA_VERSION_NUM >= 502
		lua_State* L = state->getState();
		lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
		int global = lua_gettop(L);
		get(key, global);
		lua_remove(L, global);
#else
#error unknown Lua version
#endif
		return *this;
	}

	Stack& getGlobal(const std::string& key) { return getGlobalType<std::string>(key); }
	Stack& getGlobal(int key) { return getGlobalType<int>(key); }
	Stack& getGlobal(float key) { return getGlobalType<float>(key); }
	Stack& getGlobal(double key) { return getGlobalType<double>(key); }

	//performs a lua call
	Stack& call(int nargs, int nret);

	//get top location
	int top();
};

template<>
inline Stack& Stack::pop() {
	lua_pop(state->getState(), 1);
	return *this;
}

}

