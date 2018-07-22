#pragma once

#include <lua.hpp>

namespace LuaCxx {

template<typename T>
struct fromC_impl {
	//static void exec(lua_State* L, const T& value) {}
};

template<>
struct fromC_impl<bool> {
	static void exec(lua_State* L, bool value) {
		lua_pushboolean(L, value);
	}
};

template<>
struct fromC_impl<int> {
	static void exec(lua_State* L, int value) {
		lua_pushinteger(L, value);
	}
};

template<>
struct fromC_impl<float> {
	static void exec(lua_State* L, float value) {
		lua_pushnumber(L, value);
	}
};

template<>
struct fromC_impl<double> {
	static void exec(lua_State* L, double value) {
		lua_pushnumber(L, value);
	}
};

template<>
struct fromC_impl<const char*> {
	static void exec(lua_State* L, const char* value) {
		lua_pushstring(L, value);
	}
};

template<>
struct fromC_impl<std::string> {
	static void exec(lua_State* L, std::string value) {
		lua_pushlstring(L, value.c_str(), value.size());
	}
};

template<typename T>
inline void fromC(lua_State* L, const T& value) {
	fromC_impl<T>::exec(L, value);
}

}
