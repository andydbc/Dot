function main(x, y, frame)
	v = x + frame
	if(math.fmod(v, 14) == 0) then
		return 1
	end
	return 0
end