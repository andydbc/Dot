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

bool interpreter::run_from_memory(const std::string& code)
{
	return luaL_dostring(_lua.get(), code.c_str())==0;
}

bool interpreter::run_from_file(const std::string& filepath)
{
	return luaL_dofile(_lua.get(), filepath.c_str())==0;
}

int interpreter::wrapper(StatePtr state)
{
	auto* lambda = (std::function<int(StatePtr)>*)lua_touserdata(state, lua_upvalueindex(1));
	return (*lambda)(state);
}

DOT_NS_END