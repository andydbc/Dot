#pragma once
#include <vector>

class display
{
public:
	display(int w, int h) : _w(w), _h(h)
	{
		_pixels.resize(w * h);
	}

	void set_pixel(int x, int y, int v)
	{
		int idx = x + _w * y;
		_pixels[idx] = v;
	}

	int get_pixel(int x, int y)
	{
		int idx = x + _w * y;
		return _pixels[idx];
	}

	std::vector<int>& pixels() 
	{
		return _pixels;
	}

private:
	int _w, _h;
	std::vector<int> _pixels;
};