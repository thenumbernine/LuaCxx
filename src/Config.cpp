#include "Config/Config.h"
#include "Common/Exception.h"
#include "Common/File.h"
#include <functional>

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

Config &Config::loadFile(std::string filename) {
	std::string str = Common::File::read(filename);
	return loadString(str);
}

Config &Config::loadString(std::string str) {
	std::function<int(lua_State*)> errorHandler = [&](lua_State *L) -> int {
		std::cout << "lua error " << lua_tostring(L, -1) << std::endl;
		return 0;
	};
	lua_pushcfunction(L, errorHandler.target<int(lua_State*)>());
	luaL_loadstring(L, str.c_str());
	lua_pcall(L, 0, 0, lua_gettop(L)-1);
	lua_pop(L,1);	//remove error handler
	return *this;
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

