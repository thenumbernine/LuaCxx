#include "LuaCxx/State.h"
#include "LuaCxx/Ref.h"
#include "LuaCxx/GlobalTable.h"
#include "LuaCxx/Stack.h"
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
	std::ostringstream ss;
	ss << "lua error\n" << lua_tostring(L, -1) << "\n";
	lua_getglobal(L, "debug");	//debug
	lua_getfield(L, lua_gettop(L), "traceback");	//debug, debug.traceback
	lua_remove(L, lua_gettop(L)-1);	//debug.traceback
	lua_call(L, 0, 1);	//traceback-results
	ss << lua_tostring(L, -1);
	lua_pop(L,1);
	throw Common::Exception() << ss.str();
}

State& State::loadFile(const std::string& filename) {
	luaL_loadfile(L, filename.c_str());
	if (!lua_isfunction(L, lua_gettop(L))) throw Common::Exception() << "failed to load file " << filename << " with error " << lua_tostring(L,-1);
	call(0,0);
	return *this;
}

State& State::loadString(const std::string& str) {
	return runString(str, 0, 0);
}

State& State::runString(const std::string& str, int narg, int nret) {
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
	
GlobalTable State::ref() { 
	return GlobalTable(this); 
}

Stack State::stack() {
	return Stack(this);
}

//must have one for each instance of Ref::operator[]
template<> Ref State::operator[](int key) { return ref()[key]; }
template<> Ref State::operator[](float key) { return ref()[key]; }
template<> Ref State::operator[](double key) { return ref()[key]; }
template<> Ref State::operator[](const char* key) { return ref()[key]; }
template<> Ref State::operator[](const std::string& key) { return ref()[key]; }

State& State::operator<<(const std::string& str) {
	loadString(str);
	return *this;
}

}
