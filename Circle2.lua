dmath = require "dot_math"

cur_effect = 1

function on_midi_input(state, id)
	if state == 128 then
		return
	end

	if id == 40 then
		cur_effect = 0
	elseif id == 41 then
		cur_effect = 1
	else
		cur_effect = 2
	end
end

function fill()
	return 1
end

function clear()
	return 0
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
	if cur_effect == 0 then
		return fill()
	elseif cur_effect == 1 then
		return clear()
	else
		return circle(x,y,frame)
	end
end