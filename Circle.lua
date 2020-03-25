function dist(x1, y1, x2, y2)
	return ((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2))
end

function main(x, y, frame)

	t = math.fmod(frame*20, 300)
	d = dist(x, y, 6.5, 13.5)
	--d = math.fmod(d, 60)

	val = 0
	if(d > t-20 and d < t+20) then
		val = 1
	end
	return val
end