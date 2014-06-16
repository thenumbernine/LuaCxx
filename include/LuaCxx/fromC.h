#pragma once

#include <lua.hpp>

namespace LuaCxx {

template<typename T>
inline void fromC(lua_State* L, const T& value);

template<>
inline void fromC<bool>(lua_State* L, const bool& value) {
	lua_pushboolean(L, value);
}

template<>
inline void fromC<int>(lua_State* L, const int& value) {
	lua_pushinteger(L, value);
}

template<>
inline void fromC<float>(lua_State* L, const float& value) {
	lua_pushnumber(L, value);
}

template<>
inline void fromC<double>(lua_State* L, const double& value) {
	lua_pushnumber(L, value);
}

template<>
inline void fromC<std::string>(lua_State* L, const std::string& value) {
	lua_pushlstring(L, value.c_str(), value.size());
}

};

