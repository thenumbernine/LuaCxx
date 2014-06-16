#pragma once

#include <string>
#include <lua.hpp>

namespace LuaCxx {

template<typename T> inline T toC(lua_State *L, int loc);
template<> inline bool toC<bool>(lua_State *L, int loc) { return lua_toboolean(L, loc); }
template<> inline int toC<int>(lua_State *L, int loc) { return lua_tointeger(L, loc); }
template<> inline float toC<float>(lua_State *L, int loc) { return (float)lua_tonumber(L, loc); }
template<> inline double toC<double>(lua_State *L, int loc) { return lua_tonumber(L, loc); }
template<> inline std::string toC<std::string>(lua_State *L, int loc) { 
	size_t len = 0;
	const char *cResult = lua_tolstring(L, loc, &len);
	return std::string(cResult, len);
}

};

