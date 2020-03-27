#pragma once

#include <vector>

class perlin
{
public:
	static void set_seed(unsigned int seed);
	static double noise(double x, double y, double z);
private:
	static double fade(double t);
	static double lerp(double t, double a, double b);
	static double grad(int hash, double x, double y, double z);

	static std::vector<int> p;
};

struct lua_State;
void register_perlin(lua_State* lua);