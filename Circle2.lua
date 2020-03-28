function dist(x1, y1, x2, y2)
	x = x1-x2
	y = y1-y2
	xx = x*x
	yy = y*y
	return math.sqrt(xx+yy)
end

function main(x, y, frame)

	c_x = math.sin(frame * 0.01) * 10
	c_y = math.cos(frame * 0.01) * 10
	d = dist(x + c_x, y + c_y, 14, 14)
	d = d*5
	
	val = 1
	if(math.fmod(d - frame, 75) > 25) then
		val = 0
	end
	return val
end