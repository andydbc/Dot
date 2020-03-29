#pragma once

#include "common.h"

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <tuple>

extern "C" {
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
}

using State = lua_State;
typedef State* StatePtr;

DOT_NS_BEGIN

class interpreter
{
public:

	interpreter();

	bool run_from_memory(const std::string& code);
	bool run_from_file(const std::string& filepath);
	
	template<typename To, typename ...Ts>
	void add(const std::string& name, To function(Ts...))
	{
		std::function<int(StatePtr state)> lambda;
		lambda = [this, function](StatePtr state)
		{	
			To r = function(Ts...);
			this->push(r);
			return 1;
		};

		_functions[name] = lambda;

		lua_pushlightuserdata(_lua.get(), &_functions[name]);
		lua_pushcclosure(_lua.get(), interpreter::wrapper, 1);
		lua_setglobal(_lua.get(), name.c_str());
	}

	template<typename To, typename ...Ts>
	To call(const std::string& name, Ts... args)
	{
		lua_getglobal(_lua.get(), name.c_str());
		auto args_tuple = std::tuple<Ts...>(args...);
		push(args_tuple);
		auto args_count = sizeof...(args);

		if (lua_pcall(_lua.get(), args_count, 1, 0))
		{
			std::cout << lua_tostring(_lua.get(), -1) << std::endl;
		}

		return get<To>();
	}

private:

	static int wrapper(StatePtr state);

	template<std::size_t I = 0, typename... Tp>
	inline typename std::enable_if<I == sizeof...(Tp), void>::type
		push(std::tuple<Tp...>& t) { }

	template<std::size_t I = 0, typename... Tp>
	inline typename std::enable_if < I < sizeof...(Tp), void>::type
		push(std::tuple<Tp...>& t)
	{
		push(std::get<I>(t));
		push<I + 1, Tp...>(t);
	}

	void push(void) { }
	void push(int v) { lua_pushinteger(_lua.get(), v); }
	void push(float v) { lua_pushnumber(_lua.get(), v); }

	template<typename T>
	T get() { }

	template<>
	int get() 
	{ 
		int value = lua_tointeger(_lua.get(), -1);
		lua_pop(_lua.get(), 1);
		return value;
	}

	template<>
	float get()
	{
		float value = lua_tonumber(_lua.get(), -1);
		lua_pop(_lua.get(), 1);
		return value;
	}

private:

	std::shared_ptr<State> _lua;
	std::map<std::string, std::function<int(StatePtr)>> _functions;
};

DOT_NS_END