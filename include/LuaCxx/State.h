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
	lua_State* L;

	static int errorHandler(lua_State*);
	
public:
	State();
	virtual ~State();

	State& loadFile(const std::string& filename);
	State& loadString(const std::string& str);
	State& runString(const std::string& str, int narg, int nret);

	//expects function and args on the stack 
	//returns with results on the stack
	int call(int nargs, int nresults);

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
};

}
