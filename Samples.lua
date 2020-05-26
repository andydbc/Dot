dmath = require "dot_math"

-- Constants
FILL = 0
CLEAR = 1
CIRCLES = 2
DISTANCE_FIELD = 3

cur_effect = 2

function on_midi_input(state, id)
	if state == 128 then
		return
	end

	if id == 40 then
		cur_effect = FILL
	elseif id == 41 then
		cur_effect = CLEAR
	elseif id == 42 then
		cur_effect = CIRCLES
	elseif id == 43 then
		cur_effect = DISTANCE_FIELD
	end
end

function fill()
	return 1
end

function clear()
	return 0
end

function distance_field(x,y,f)
	c_x = math.sin(23+f * 0.1) * 15
	c_y = math.cos(444+f * 0.1) * 14
	d = dmath.dist(x + c_x, y + c_y, 14, 14)

	c2_x = math.sin(f * 0.1) * 10
	c2_y = math.sin(f * 0.1) * 15
	d2= dmath.dist(x + c2_x, y + c2_y, 14, 14)

	c3_x = math.sin(12-f * 0.05) * 11
	c3_y = math.sin(222+-f * 0.1) * 20
	d3= dmath.dist(x + c3_x, y + c3_y, 14, 14)

	c4_x = math.sin(1442+f * 0.09) * 11
	c4_y = math.sin(25+f * 0.05) * 12
	d4= dmath.dist(x + c4_x, y + c4_y, 14, 14)

	d = dmath.smin(dmath.smin(dmath.smin(d, d2, 12), d3, 12), d4, 12)
	
	val = 0
	if(d < 4.5) then
		val = 1
	end
	return val
end

function circle(x, y, f)
	c_x = math.sin(f * 0.01) * 15
	c_y = math.cos(f * 0.01) * 15
	d = dmath.dist(x + c_x, y + c_y, 7, 14)
	d = d*6
	
	val = 1
	if(math.fmod(f * 6 - d, 100) > 60) then
		val = 0
	end
	return val
end

function main(x, y, frame)
	if cur_effect == FILL then
		return fill()
	elseif cur_effect == CLEAR then
		return clear()
	elseif cur_effect == CIRCLES then
		return circle(x,y,frame)
	elseif cur_effect == DISTANCE_FIELD then
		return distance_field(x,y,frame)
	end
end