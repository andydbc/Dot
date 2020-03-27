#include "interpreter.h"

DOT_NS_BEGIN

interpreter::interpreter()
{
	_lua = std::shared_ptr<State>(
		luaL_newstate(),
		lua_close
	);

	luaL_openlibs(_lua.get());
}

void interpreter::run_from_memory(const std::string& code)
{
	luaL_dostring(_lua.get(), code.c_str());
}

void interpreter::run_from_file(const std::string& filepath)
{
	luaL_dofile(_lua.get(), filepath.c_str());
}

DOT_NS_END