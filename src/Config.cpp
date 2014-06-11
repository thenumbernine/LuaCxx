#include "Config/Config.h"
#include "Common/Exception.h"

#include <functional>

#include "lauxlib.h"
#include "lualib.h"

namespace Config {

Config::Config(std::string filename) 
: L(luaL_newstate())
{
	lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
	luaL_openlibs(L);
	lua_gc(L, LUA_GCRESTART, -1);

	//http://luajit.org/ext_c_api.html
	
	typedef int ExceptionWrapper(lua_State *L, lua_CFunction f);
	
	std::function<ExceptionWrapper> wrapper = [&](lua_State *L, lua_CFunction f) -> int {
		try {
			return f(L);
		} catch (const char *s) {  // Catch and convert exceptions.
			lua_pushstring(L, s);
		} catch (std::exception& e) {
			lua_pushstring(L, e.what());
		} catch (...) {
			lua_pushliteral(L, "caught (...)");
		}
		return lua_error(L);  // Rethrow as a Lua error.
	};

	lua_pushlightuserdata(L, (void*)wrapper.target<ExceptionWrapper>());
	luaJIT_setmode(L, -1, LUAJIT_MODE_WRAPCFUNC | LUAJIT_MODE_ON);
	lua_pop(L, 1);

	loadFile(filename);
}

Config::~Config() {
	if (L) lua_close(L);
}

Config &Config::loadFile(std::string filename) {
	if (!luaL_dofile(L, filename.c_str())) throw Common::Exception() << "failed to load file " << filename;
	return *this;
}

Config &Config::loadString(std::string str) {
	if (!luaL_dostring(L, str.c_str())) throw Common::Exception() << "failed to load string " << str;
	return *this;
}

};

