#pragma once

#include "LuaCxx/GlobalTable.h"
#include "Common/Exception.h"
#include <lua.hpp>
#include <string>
#include <functional>

namespace LuaCxx {

struct State {
protected:
	lua_State *L;

	static int errorHandler(lua_State *);
	
public:
	State();
	virtual ~State();

	State& loadFile(const std::string& filename);
	State& loadString(const std::string& str);
	State& runString(const std::string& str, int narg, int nret);

	//expects function and args on the stack 
	//returns with results on the stack
	int call(int nargs, int nresults);

	GlobalTable _G() { return GlobalTable(L); }
	lua_State *getState() { return L; }
};

}

