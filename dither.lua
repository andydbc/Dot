dmath = require "dot_math"

local bayer = {}
bayer[0] = {}
bayer[1] = {}
bayer[2] = {}
bayer[3] = {}

bayer[0][0] = 0
bayer[0][1] = 0.5
bayer[0][2] = 0.125
bayer[0][3] = 0.625
bayer[1][0] = 0.75
bayer[1][1] = 0.25 
bayer[1][2] = 0.875
bayer[1][3] = 0.375
bayer[2][0] = 0.1875
bayer[2][1] = 0.6875
bayer[2][2] = 0.0625
bayer[2][3] = 0.5625
bayer[3][0] = 0.9375
bayer[3][1] = 0.4375
bayer[3][2] = 0.8125
bayer[3][3] = 0.3125

function circle(x, y, cx, cy)
	d = dmath.dist(x, y, cx, cy)
	return d
end

function dither(x,y,b)
	mx = math.fmod(x,4)
	my = math.fmod(y,4)
	if(b < bayer[mx][my]) then
		return 1
	end
	return 0
end

function simple(x,y,f)
	b = (math.sin(((x / 24) + f * 0.075) * 5) + 1.0) / 2.0
	return dither(x,y,b)
end

function main(x, y, frame)
	return simple(x,y,frame)
end