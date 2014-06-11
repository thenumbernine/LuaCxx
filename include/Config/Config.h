#pragma once

#include <string>

extern "C" {
#include <luajit.h>
#include <lua.h>
}

namespace Config {

struct Config {
protected:
	lua_State *L;

public:
	Config(std::string filename = "config.lua");
	virtual ~Config();

	Config &loadFile(std::string filename);
	Config &loadString(std::string str);

	template<typename T> T get(std::string name);
};


template<> double Config::get<double>(std::string name) {
	lua_getglobal(L, name.c_str());
	double result = lua_tonumber(L, -1);
	lua_pop(L,1);
	return result;
}

template<> std::string Config::get<std::string>(std::string name) {
	lua_getglobal(L, name.c_str());
	size_t len = 0;
	const char *cResult = lua_tolstring(L, -1, &len);
	std::string result(cResult, len);
	lua_pop(L,1);
	return result;
}

}

