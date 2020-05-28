dmath = require "dot_math"
matrix = require "matrix"

-- Constants
FILL = 0
CLEAR = 1
CIRCLES = 2
DISTANCE_FIELD = 3
QUADS_0 = 4
QUADS_1 = 5

cur_effect = DISTANCE_FIELD

function on_midi_input(state, id)
	print(id)
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
	elseif id == 36 then
		cur_effect = QUADS_0
	elseif id == 37 then
		cur_effect = QUADS_1
	end
end

function fill()
	return 1
end

function clear()
	return 0
end

function distance_field(x,y,f)
	s0 = 0.05
	c_x = math.sin(23+f * s0) * 15
	c_y = math.cos(444+f * s0) * 14
	d = dmath.dist(x + c_x, y + c_y, 14, 14)

	c2_x = math.sin(f * s0) * 10
	c2_y = math.sin(f * s0) * 15
	d2= dmath.dist(x + c2_x, y + c2_y, 14, 14)
	
	s1 = 0.05 
	c3_x = math.sin(12-f * s1) * 11
	c3_y = math.sin(222+-f * s1) * 20
	d3= dmath.dist(x + c3_x, y + c3_y, 14, 14)

	s2 = 0.09
	c4_x = math.sin(1442+f * s2) * 11
	c4_y = math.sin(25+f * s2 * 0.5) * 12
	d4= dmath.dist(x + c4_x, y + c4_y, 14, 14)

	d = dmath.smin(dmath.smin(dmath.smin(d, d2, 12), d3, 12), d4, 12)
	
	val = 0
	if(d < 4.5) then
		val = 1
	end
	return val
end

function circle(x, y, f)
	c_x = 0--math.sin(f * 0.01) * 15
	c_y = 0--math.cos(f * 0.01) * 15
	d = dmath.dist(x + c_x, y + c_y, 7, 14)
	d = d*20
	
	val = 1
	if(math.fmod(f * 6 - d, 120) > 60) then
		val = 0
	end
	return val
end

function sdBox( x,y, w,h )
	dx = math.abs(x)-w;
	dy = math.abs(y)-h;
	if dx < 0 then
		dx = 0
	end
	if dy < 0 then
		dy = 0
	end
	
	m = math.max(dx, dy)
	lx = dx + math.min(m, 0.0)
	ly = dy + math.min(m, 0.0)
	return math.sqrt(lx*lx + ly*ly)
end

r = matrix(2,2)
p = matrix {0,0}

function draw_quad(px, py, f, x, y, w, h, d)
	c = math.cos(d)
	s = math.sin(d)

	-- rotation matrix
	matrix.setelement(r,1,1,c)
	matrix.setelement(r,1,2,-s)
	matrix.setelement(r,2,1,s)
	matrix.setelement(r,2,2,c)
	matrix.setelement(p,1,1,px-x)
	matrix.setelement(p,2,1,py-y)
	
	wp = matrix.invert(r) * p
	v = sdBox(wp[1][1], wp[2][1], w, h)
	if v < 0.5 then
		return 1
	end
	return 0
end

function quads2(x,y,f)
	q0 = draw_quad(x,y,f, 6.5, 14, 7, 14, f*0.1)
	--q1 = draw_quad(x,y,f, 6.5, 14, 1, 207, 90-f*0.1)
	--q2 = draw_quad(x,y,f, 6.5, 14, 0.7, 20, 75+f*0.05)
	return q0--math.max(q0, q1)
end

function quads(x,y,f)
	h = math.sin((x + 0.15) * 0.8 - f * 0.1)
	v = math.sin(y * 0.45 - f * 0.21)
	if h < 0 then
		h = 0
	else
		h = 1
	end
	if v < 0 then
		v = 0
	else
		v = 1
	end
	


	return math.max(h,v)
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
	elseif cur_effect == QUADS_0 then
		return quads(x,y,frame)
	elseif cur_effect == QUADS_1 then
		return quads2(x,y,frame)
	end
end