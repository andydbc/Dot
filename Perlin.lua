function main(x, y, frame)
	if (frame==0) then
		perlin_seed(4439)
	end

	ux = x/14.0 * 2.0
	uy = y/28.0 * 2.0 - (frame * 0.2)

	v = perlin_noise(ux, uy, 0) * 100
	if(v > 50) then
		return 1
	end
	return 0
end