#pragma once

#include "Common/Exception.h"
#include <lua.hpp>
#include <string>
#include <functional>

namespace LuaCxx {

struct GlobalTable;
struct Stack;
struct Ref;

struct State {
protected:
	lua_State * L = {};
	bool owns = {};

	static int errorHandler(lua_State *);
	
public:
	// create a state, be responsible for cleaning it up
	State();
	
	// create a wrapper for an existing state
	State(lua_State * L_);

	// cleanup
	virtual ~State();
	
	//executes a string via a lua_call, with narg and nret on the stack
	State& runString(const std::string& str, int narg, int nret);

	//executes a string via runString.  no arguments, no return.
	State& loadString(const std::string& str);

	//laods the file and executes it via lua_loadfile 
	State& loadFile(const std::string& filename);

	//expects function and args on the stack 
	//returns with results on the stack
	void call(int nargs, int nresults);

	lua_State *getState() { return L; }

	//reference to global table - base for all subsequent references
	GlobalTable ref();

	Stack stack();

	//shorthand for lua.ref()[...]
	//tell me if this is unintuitive or a bad idea
	//template<typename T> Ref operator[](T key) { return lua.ref()[key]; }
	//...but I can't just yet because of include circular dependencies
	//...that might not be able to resolve until I separate struct prototypes from inline header bodies (.h vs .hpp)
	//in the mean time, I can still add individual template bodies to the .cpp file
	template<typename T> Ref operator[](T key);

	//shorthand for loadString
	State& operator<<(const std::string& str);
};

}
