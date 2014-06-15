#include "Config/Config.h"
#include "Common/File.h"

namespace Config {

Config::Config() 
{
	L = luaL_newstate();
	if (!L) throw Common::Exception() << "failed to create lua state!";
	luaL_openlibs(L);
}

Config::~Config() {
	if (L) lua_close(L);
}
	
int Config::errorHandler(lua_State *L) {
	throw Common::Exception() << "lua error " << lua_tostring(L, -1);
}

Config &Config::loadFile(const std::string& filename) {
	luaL_loadfile(L, filename.c_str());
	if (!lua_isfunction(L, lua_gettop(L))) throw Common::Exception() << "failed to load file " << filename << " with error " << lua_tostring(L,-1);
	call(0,0);
	return *this;
}

Config &Config::loadString(const std::string& str) {
	return runString(str, 0, 0);
}
	
Config &Config::runString(const std::string &str, int narg, int nret) {
	luaL_loadstring(L, str.c_str());
	if (!lua_isfunction(L, lua_gettop(L))) throw Common::Exception() << "failed to load string " << str << " with error " << lua_tostring(L, -1);
	call(narg, nret);
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

bool Config::get(const std::string& name, bool &result) {
	lua_getglobal(L, name.c_str());
	if (lua_isnil(L, -1)) {
		lua_pop(L,1);
		return false;
	}
	result = luaToC<bool>(L, -1);
	lua_pop(L,1);
	return true;
}

bool Config::get(const std::string& name, int &result) {
	lua_getglobal(L, name.c_str());
	if (lua_isnil(L, -1)) {
		lua_pop(L,1);
		return false;
	}
	result = luaToC<int>(L, -1);
	lua_pop(L,1);
	return true;
}

bool Config::get(const std::string& name, float &result) {
	double doubleResult;
	if (!get(name, doubleResult)) return false;
	result = (float)doubleResult;
	return true;
}

bool Config::get(const std::string& name, double &result) {
	lua_getglobal(L, name.c_str());
	if (lua_isnil(L, -1)) {
		lua_pop(L,1);
		return false;
	}
	result = luaToC<double>(L, -1);
	lua_pop(L,1);
	return true;
}

bool Config::get(const std::string& name, std::string &result) {
	lua_getglobal(L, name.c_str());
	if (lua_isnil(L, -1)) {
		lua_pop(L,1);
		return false;
	}
	result = luaToC<std::string>(L, -1);
	lua_pop(L,1);
	return true;
}

};

