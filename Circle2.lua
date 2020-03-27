function dist(x1, y1, x2, y2)
	return ((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2))
end

function main(x, y, frame)

	c_x = math.sin(frame * 0.01) * 8
	c_y = math.cos(frame * 0.01) * 8
	d = dist(x + c_x, y + c_y, 6.5, 12)
	d = d * 0.2
	
	val = 1
	if(math.fmod(d - frame, 75) > 25) then
		val = 0
	end
	return val
end