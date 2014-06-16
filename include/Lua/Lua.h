#pragma once

#include "Lua/GlobalTable.h"
#include "Common/Exception.h"
#include <lua.hpp>
#include <string>
#include <functional>

namespace Lua {

struct Lua {
protected:
	lua_State *L;

	static int errorHandler(lua_State *);
	
public:
	Lua();
	virtual ~Lua();

	Lua& loadFile(const std::string& filename);
	Lua& loadString(const std::string& str);
	Lua& runString(const std::string& str, int narg, int nret);

	//expects function and args on the stack 
	//returns with results on the stack
	int call(int nargs, int nresults);

	GlobalTable _G() { return GlobalTable(L); }
	lua_State *getState() { return L; }
};

}

