#pragma once

#include "Common/Exception.h"
#include <lua.hpp>
#include <string>
#include <functional>

namespace LuaCxx {

struct GlobalTable;
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
};

}

