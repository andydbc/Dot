function dist(x1, y1, x2, y2)
	x = x1-x2
	y = y1-y2
	return math.sqrt(x*x+y*y)
end

function clamp(n, low, high) 
	return math.min(math.max(n, low), high) 
end

function lerp(a, b, t)
	return a + (b - a) * t
end

function min(d1, d2, k)
	h = clamp(0.5 + 0.5 * (d2-d1) / k, 0.0, 1.0)
	return lerp(d2, d1, h) - k * h * (1.0-h)
end

function main(x, y, frame)
	c_x = math.sin(23+frame * 0.01) * 15
	c_y = math.cos(444+frame * 0.01) * 14
	d = dist(x + c_x, y + c_y, 14, 14)
	d = d

	c2_x = math.sin(frame * 0.01) * 10
	c2_y = math.sin(frame * 0.01) * 15
	d2= dist(x + c2_x, y + c2_y, 14, 14)

	c3_x = math.sin(12-frame * 0.005) * 11
	c3_y = math.sin(222+-frame * 0.01) * 20
	d3= dist(x + c3_x, y + c3_y, 14, 14)

	c4_x = math.sin(1442+frame * 0.009) * 11
	c4_y = math.sin(25+frame * 0.005) * 12
	d4= dist(x + c4_x, y + c4_y, 14, 14)

	d = min(min(min(d, d2, 12), d3, 12), d4, 12)
	
	val = 0
	if(d < 4.5) then
		val = 1
	end
	return val
end