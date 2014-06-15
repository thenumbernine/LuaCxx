#pragma once

#include <string>
#include <functional>
#include "Common/Exception.h"
#include "lua.hpp"

namespace Config {

template<typename T> inline T luaToC(lua_State *L, int loc);
template<> inline bool luaToC<bool>(lua_State *L, int loc) { return lua_toboolean(L, loc); }
template<> inline int luaToC<int>(lua_State *L, int loc) { return lua_tointeger(L, loc); }
template<> inline float luaToC<float>(lua_State *L, int loc) { return (float)lua_tonumber(L, loc); }
template<> inline double luaToC<double>(lua_State *L, int loc) { return lua_tonumber(L, loc); }
template<> inline std::string luaToC<std::string>(lua_State *L, int loc) { 
	size_t len = 0;
	const char *cResult = lua_tolstring(L, loc, &len);
	return std::string(cResult, len);
}

struct Config {
protected:
	lua_State *L;

	static int errorHandler(lua_State *);
	
public:
	Config();
	virtual ~Config();

	Config &loadFile(const std::string &filename);
	Config &loadString(const std::string &str);
	Config &runString(const std::string &str, int narg, int nret);

	//expects function and args on the stack 
	//returns with results on the stack
	int call(int nargs, int nresults);

	bool get(const std::string& name, bool &result);
	bool get(const std::string& name, int &result);
	bool get(const std::string& name, float &result);
	bool get(const std::string& name, double &result);
	bool get(const std::string& name, std::string &result);

	//I would like a function for execute-and-return
	// but what return type should we allow?
	// restrict to only one value for C-compatibility's sake?
	// or pass vectors for arguments and for return values?
	template<typename T>
	T run(const std::string &code) {
		runString(code, 0, 1);
		T result = luaToC<T>(L, -1);
		lua_pop(L,1);
		return result;
	}

	lua_State *getState() { return L; }
};

}

