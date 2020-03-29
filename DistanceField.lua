dot_math = require "dot_math"

function main(x, y, frame)
	c_x = math.sin(23+frame * 0.01) * 15
	c_y = math.cos(444+frame * 0.01) * 14
	d = dot_math.dist(x + c_x, y + c_y, 14, 14)
	d = d

	c2_x = math.sin(frame * 0.01) * 10
	c2_y = math.sin(frame * 0.01) * 15
	d2= dot_math.dist(x + c2_x, y + c2_y, 14, 14)

	c3_x = math.sin(12-frame * 0.005) * 11
	c3_y = math.sin(222+-frame * 0.01) * 20
	d3= dot_math.dist(x + c3_x, y + c3_y, 14, 14)

	c4_x = math.sin(1442+frame * 0.009) * 11
	c4_y = math.sin(25+frame * 0.005) * 12
	d4= dot_math.dist(x + c4_x, y + c4_y, 14, 14)

	d = dot_math.smin(dot_math.smin(dot_math.smin(d, d2, 12), d3, 12), d4, 12)
	
	val = 0
	if(d < 4.5) then
		val = 1
	end
	return val
end