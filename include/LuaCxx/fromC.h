#pragma once

#include <lua.hpp>

namespace LuaCxx {

template<typename T>
inline void fromC(lua_State* L, T value);

template<>
inline void fromC<bool>(lua_State* L, bool value) {
	lua_pushboolean(L, value);
}

template<>
inline void fromC<int>(lua_State* L, int value) {
	lua_pushinteger(L, value);
}

template<>
inline void fromC<float>(lua_State* L, float value) {
	lua_pushnumber(L, value);
}

template<>
inline void fromC<double>(lua_State* L, double value) {
	lua_pushnumber(L, value);
}

template<>
inline void fromC<const char*>(lua_State* L, const char* value) {
	lua_pushstring(L, value);
}

template<>
inline void fromC<std::string>(lua_State* L, std::string value) {
	lua_pushlstring(L, value.c_str(), value.size());
}

}
