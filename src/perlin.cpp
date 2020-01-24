#include "perlin.h"
#include <cmath>
#include <random>
#include <algorithm>
#include <numeric>

std::vector<int> perlin::p;

void perlin::set_seed(unsigned int seed)
{
	p = {
		151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
		8,99,37,240,21,10,23,190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
		35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,
		134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
		55,46,245,40,244,102,143,54, 65,25,63,161,1,216,80,73,209,76,132,187,208, 89,
		18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,
		250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
		189,28,42,223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167,
		43,172,9,129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,
		97,228,251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,
		107,49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 
	};
	p.insert(p.end(), p.begin(), p.end());
	p.resize(256);
	std::iota(p.begin(), p.end(), 0);
	std::default_random_engine engine(seed);
	std::shuffle(p.begin(), p.end(), engine);
	p.insert(p.end(), p.begin(), p.end());
}

double perlin::noise(double x, double y, double z)
{
	// Find the unit cube that contains the point
	int X = (int)floor(x) & 255;
	int Y = (int)floor(y) & 255;
	int Z = (int)floor(z) & 255;

	// Find relative x, y,z of point in cube
	x -= floor(x);
	y -= floor(y);
	z -= floor(z);

	// Compute fade curves for each of x, y, z
	double u = fade(x);
	double v = fade(y);
	double w = fade(z);

	// Hash coordinates of the 8 cube corners
	int A = p[X] + Y;
	int AA = p[A] + Z;
	int AB = p[A + 1] + Z;
	int B = p[X + 1] + Y;
	int BA = p[B] + Z;
	int BB = p[B + 1] + Z;

	// Add blended results from 8 corners of cube
	double res = lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z), grad(p[BA], x - 1, y, z)), lerp(u, grad(p[AB], x, y - 1, z), grad(p[BB], x - 1, y - 1, z))), lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1), grad(p[BA + 1], x - 1, y, z - 1)), lerp(u, grad(p[AB + 1], x, y - 1, z - 1), grad(p[BB + 1], x - 1, y - 1, z - 1))));
	return (res + 1.0) / 2.0;
}

double perlin::fade(double t) {
	return t * t * t * (t * (t * 6 - 15) + 10);
}

double perlin::lerp(double t, double a, double b) {
	return a + t * (b - a);
}

double perlin::grad(int hash, double x, double y, double z) {
	int h = hash & 15;
	// Convert lower 4 bits of hash into 12 gradient directions
	double u = h < 8 ? x : y,
		v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

static int lua_perlin_seed(lua_State* lua)
{
	int seed =  lua_tointeger(lua, 1);
	perlin::set_seed(seed);
	return 0;
}

static int lua_perlin_noise(lua_State* lua)
{
	float x = lua_tonumber(lua, 1);
	float y = lua_tonumber(lua, 2);
	float z = lua_tonumber(lua, 3);
	float r = perlin::noise(x, y, z);
	lua_pushnumber(lua, r);
	return 1;
}

static const struct luaL_Reg perlinlib[] = {
	{"perlin_seed", lua_perlin_seed},
	{"perlin_noise", lua_perlin_noise},
	{NULL, NULL}
};

void register_perlin(lua_State* lua)
{
	lua_getglobal(lua, "_G");
	luaL_setfuncs(lua, perlinlib, 0);
	lua_pop(lua, 1);
}