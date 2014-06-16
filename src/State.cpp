#include "LuaCxx/State.h"
#include "Common/File.h"

namespace LuaCxx {

State::State() 
: L(luaL_newstate())
{
	if (!L) throw Common::Exception() << "failed to create lua state!";
	luaL_openlibs(L);
}

State::~State() {
	if (L) lua_close(L);
}
	
int State::errorHandler(lua_State *L) {
	throw Common::Exception() << "lua error " << lua_tostring(L, -1);
}

State &State::loadFile(const std::string& filename) {
	luaL_loadfile(L, filename.c_str());
	if (!lua_isfunction(L, lua_gettop(L))) throw Common::Exception() << "failed to load file " << filename << " with error " << lua_tostring(L,-1);
	call(0,0);
	return *this;
}

State &State::loadString(const std::string& str) {
	return runString(str, 0, 0);
}
	
State &State::runString(const std::string &str, int narg, int nret) {
	luaL_loadstring(L, str.c_str());
	if (!lua_isfunction(L, lua_gettop(L))) throw Common::Exception() << "failed to load string " << str << " with error " << lua_tostring(L, -1);
	call(narg, nret);
	return *this;	
}

//expects function and args on the stack 
int State::call(int nargs, int nresults) {
	lua_pushcfunction(L, errorHandler);	//add error handler
	int errHandlerLoc = lua_gettop(L) - nargs - 1;
	lua_insert(L, errHandlerLoc);	//move it beneath the function and its args
	if (!lua_isfunction(L, errHandlerLoc)) throw Common::Exception() << "expected function!";
	int result = lua_pcall(L, nargs, nresults, errHandlerLoc);
	lua_remove(L, errHandlerLoc);	//remove error handler
	return result; //return with results on the stack
}

Value State::operator[](const std::string& key) {
	return _G()[key];
}

Value State::operator[](int key) {
	return _G()[key];
}

};

