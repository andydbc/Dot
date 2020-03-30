matrix = {}
matrix[0] = {}
matrix[1] = {}
matrix[2] = {}
matrix[3] = {}

matrix[0][0] = 1
matrix[0][1] = 9
matrix[0][2] = 3
matrix[0][3] = 11
matrix[1][0] = 13
matrix[1][1] = 5
matrix[1][2] = 15
matrix[1][3] = 7
matrix[2][0] = 4
matrix[2][1] = 12
matrix[2][2] = 2
matrix[2][3] = 10
matrix[3][0] = 16
matrix[3][1] = 8
matrix[3][2] = 14
matrix[3][3] = 6

function dist(x1, y1, x2, y2)
	return math.sqrt(((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)))
end

function main(x, y, f)
	nx = x / 14.0
	ny = y / 24.0
	a = 14.0 / 24.0
	r = 1.0 / 8.0
	d = dist(nx*a, ny, 0.475*a, 0.5) * 10
	d = math.fmod(d - f*0.05, 10)
	c = d + r*matrix[math.fmod(x, 4)][math.fmod(y, 4)] * 0.5
	
	if c > 2 then
		return 0
	end
	return 1
end