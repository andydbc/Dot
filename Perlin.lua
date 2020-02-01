function main(x, y, frame)
	if (frame==0) then
		perlin_seed(4439)
	end

	nx = x/14.0
	ny = y/28.0
	ux = nx*2.0
	uy = ny*2.0-(frame * 0.2)

	v = perlin_noise(ux, uy, frame*0.1)*100
	if(v > 50) then
		return 1
	end
	return 0
end