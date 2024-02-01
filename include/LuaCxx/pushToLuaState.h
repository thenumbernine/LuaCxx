#pragma once

#include <lua.hpp>

#include <ctime>
#include <string>
#include <vector>
#include <map>

namespace LuaCxx {

// lua<=5.2 and luajit don't have lua_seti so ...
#if LUA_VERSION_NUM <= 501

// from lua 5.3.6 lapi.c
// adopted to luajit api
inline bool lua_ispseudo(int const i) {
	return ((i) <= LUA_REGISTRYINDEX);
}

// from lua 5.3.6 lapi.c
// adopted to luajit api
inline int lua_absindex(lua_State * const L, int const index) {
	return (index > 0 || lua_ispseudo(index))
	   ? index
	   : lua_gettop(L) + index + 1;
}

#endif
#if LUA_VERSION_NUM <= 502
inline void lua_seti(lua_State * const L, int index, lua_Integer const key) {
	index = lua_absindex(L, index);
								// ..., value
	lua_pushinteger(L, key);	// ..., value, key
	lua_insert(L, -2);			// ..., key, value
	lua_settable(L, index);		// ...;   table[key] = value
}
#endif


//here's the default implementation:
//by default we'll execute the member's pushToLuaState() function ... if it has one


template<typename T>
struct pushToLuaState_impl {
	static void exec(lua_State* L, const T& value) {
		value.pushToLuaState(L);
	}
};


//here's the namespace-level function that calls the static method in the struct


template<typename T>
inline void pushToLuaState(lua_State* L, const T& value) {
	pushToLuaState_impl<T>::exec(L, value);
}


//here's the specializations:


template<>
struct pushToLuaState_impl<bool> {
	static void exec(lua_State* L, bool value) {
		lua_pushboolean(L, value);
	}
};

template<>
struct pushToLuaState_impl<int> {
	static void exec(lua_State* L, int value) {
		lua_pushinteger(L, value);
	}
};

template<>
struct pushToLuaState_impl<float> {
	static void exec(lua_State* L, float value) {
		lua_pushnumber(L, value);
	}
};

template<>
struct pushToLuaState_impl<double> {
	static void exec(lua_State* L, double value) {
		lua_pushnumber(L, value);
	}
};

template<>
struct pushToLuaState_impl<const char*> {
	static void exec(lua_State* L, const char* value) {
		lua_pushstring(L, value);
	}
};

template<>
struct pushToLuaState_impl<std::string> {
	static void exec(lua_State* L, std::string value) {
		lua_pushlstring(L, value.c_str(), value.size());
	}
};

template<>
struct pushToLuaState_impl<std::time_t> {
	static void exec(lua_State* L, std::time_t value) {
		lua_pushinteger(L, value);
	}
};

template<typename A, typename B>
struct pushToLuaState_impl<std::pair<A,B>> {
	static void exec(lua_State* L, const std::pair<A,B>& value) {
		lua_newtable(L);
		int t = lua_gettop(L);
		pushToLuaState<A>(L, value.first);
		lua_seti(L, t, 1);
		pushToLuaState<B>(L, value.second);
		lua_seti(L, t, 2);
	}
};

//notice, this copies the std::vector into an integer-indexed lua table
//so it goes from being 0-based to 1-based
template<typename T>
struct pushToLuaState_impl<std::vector<T>> {
	static void exec(lua_State* L, const std::vector<T>& value) {
		lua_newtable(L);	//{}
		for (int i = 0; i < (int)value.size(); ++i) {
			pushToLuaState<T>(L, value[i]);	//{} v[i]
			lua_seti(L, -2, i+1);			//{}			; {}[i+1] = v[i]
		}
	}
};

template<typename A, typename B>
struct pushToLuaState_impl<std::map<A,B>> {
	static void exec(lua_State* L, const std::map<A,B>& value) {
		lua_newtable(L);
		int t = lua_gettop(L);
		for (const std::pair<A,B> &p : value) {
			pushToLuaState<A>(L, p.first);
			pushToLuaState<B>(L, p.second);
			lua_settable(L, t);
		}
	}
};


}
