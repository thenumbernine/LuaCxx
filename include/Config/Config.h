#pragma once

#include <string>
#include "lua.hpp"

namespace Config {

struct Config {
protected:
	lua_State *L;

public:
	Config(std::string filename = "config.lua");
	virtual ~Config();

	Config &loadFile(std::string filename);
	Config &loadString(std::string str);

	bool get(std::string name, bool &result);
	bool get(std::string name, int &result);
	bool get(std::string name, float &result);
	bool get(std::string name, double &result);
	bool get(std::string name, std::string &result);
};

}

