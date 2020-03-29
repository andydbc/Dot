local lib = {}

function lib.dist(x1, y1, x2, y2)
	x = x1-x2
	y = y1-y2
	return math.sqrt(x*x+y*y)
end

function lib.clamp(n, low, high) 
	return math.min(math.max(n, low), high) 
end

function lib.lerp(a, b, t)
	return a + (b - a) * t
end

function lib.smooth_min(d1, d2, k)
	h = clamp(0.5 + 0.5 * (d2-d1) / k, 0.0, 1.0)
	return lerp(d2, d1, h) - k * h * (1.0-h)
end

return lib