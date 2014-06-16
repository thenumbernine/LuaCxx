#pragma once

#include "Common/Meta.h"
#include <string>
#include <lua.hpp>

namespace LuaCxx {

template<typename T> 
inline T toC(lua_State* L, int loc);

template<> 
inline bool toC<bool>(lua_State* L, int loc) {
	return lua_toboolean(L, loc); 
}

template<> 
inline int toC<int>(lua_State* L, int loc) {
	return lua_tointeger(L, loc); 
}

template<> 
inline float toC<float>(lua_State* L, int loc) {
	return (float)lua_tonumber(L, loc); 
}

template<> 
inline double toC<double>(lua_State* L, int loc) {
	return lua_tonumber(L, loc); 
}

template<> 
inline std::string toC<std::string>(lua_State* L, int loc) {
	size_t len = 0;
	const char *cResult = lua_tolstring(L, loc, &len);
	return std::string(cResult, len);
}

//I can't use toC for functions if they take in stack locations
// because there's no guarantee that the stack object will stick around.
//I could rewrite all other types to accept refs, but that would mean extra pushes and pops of values to test for nil.
// I could rethink nil handling of all types, and all conversions for that matter.
//For now I'll specialize the Value operators for functions
// and not support toC<std::function<()>>.

};

