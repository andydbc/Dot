
local lib = {}

local matrix = {}
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

function lib.dither(x, y, r, f)
   -- r = 1.0 / 8.0
    xx = math.fmod(x, 4)
    yy = math.fmod(y, 4)
    return r * matrix[xx][yy] * f
end

return lib