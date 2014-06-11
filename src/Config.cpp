#include "Config/Config.h"
#include "Common/Exception.h"
#include "Common/File.h"

namespace Config {

Config::Config(std::string filename) 
{
	L = luaL_newstate();
	if (!L) throw Common::Exception() << "failed to create lua state!";
	
	luaL_openlibs(L);

	loadFile(filename);
}

Config::~Config() {
	if (L) lua_close(L);
}
	
int Config::errorHandler(lua_State *L) {
	throw Common::Exception() << "lua error " << lua_tostring(L, -1);
}

Config &Config::loadFile(std::string filename) {
	std::string str = Common::File::read(filename);
	return loadString(str);
}

Config &Config::loadString(std::string str) {
	luaL_loadstring(L, str.c_str());
	if (!lua_isfunction(L, lua_gettop(L))) throw Common::Exception() << "expected function!";
	call(0, 0);
	return *this;
}

//expects function and args on the stack 
int Config::call(int nargs, int nresults) {
	lua_pushcfunction(L, errorHandler);	//add error handler
	int errHandlerLoc = lua_gettop(L) - nargs - 1;
	lua_insert(L, errHandlerLoc);	//move it beneath the function and its args
	if (!lua_isfunction(L, errHandlerLoc)) throw Common::Exception() << "expected function!";
	int result = lua_pcall(L, nargs, nresults, errHandlerLoc);
	lua_remove(L, errHandlerLoc);	//remove error handler
	return result; //return with results on the stack
}

bool Config::get(std::string name, bool &result) {
	lua_getglobal(L, name.c_str());
	if (lua_isnil(L, -1)) {
		lua_pop(L,1);
		return false;
	}
	result = lua_toboolean(L, -1);
	lua_pop(L,1);
	return true;
}

bool Config::get(std::string name, int &result) {
	lua_getglobal(L, name.c_str());
	if (lua_isnil(L, -1)) {
		lua_pop(L,1);
		return false;
	}
	result = lua_tointeger(L, -1);
	lua_pop(L,1);
	return true;
}

bool Config::get(std::string name, float &result) {
	double doubleResult;
	if (!get(name, doubleResult)) return false;
	result = (float)doubleResult;
	return true;
}

bool Config::get(std::string name, double &result) {
	lua_getglobal(L, name.c_str());
	if (lua_isnil(L, -1)) {
		lua_pop(L,1);
		return false;
	}
	result = lua_tonumber(L, -1);
	lua_pop(L,1);
	return true;
}

bool Config::get(std::string name, std::string &result) {
	lua_getglobal(L, name.c_str());
	if (lua_isnil(L, -1)) {
		lua_pop(L,1);
		return false;
	}
	size_t len = 0;
	const char *cResult = lua_tolstring(L, -1, &len);
	result = std::string(cResult, len);
	lua_pop(L,1);
	return true;
}

};

