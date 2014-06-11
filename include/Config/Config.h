#pragma once

#include <string>
#include <functional>
#include "lua.hpp"

namespace Config {

struct Config {
protected:
	lua_State *L;

	static int errorHandler(lua_State *);
	
public:
	Config(std::string filename = "config.lua");
	virtual ~Config();

	Config &loadFile(std::string filename);
	Config &loadString(std::string str);

	//expects function and args on the stack 
	//returns with results on the stack
	int call(int nargs, int nresults);

	bool get(std::string name, bool &result);
	bool get(std::string name, int &result);
	bool get(std::string name, float &result);
	bool get(std::string name, double &result);
	bool get(std::string name, std::string &result);

	lua_State *getState() { return L; }
};

}

