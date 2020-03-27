#include "controller.h"

#include <fstream>
#include <iostream>

static size_t buffer_length = 1024 * 10;
static char defaultScript[1024] = "-- welcome to Dot - v0.1 \n\n"
	"function all_whites()\n"
	"	return 1 \n"
	"end\n\n"
	"function all_blacks()\n"
	"	return 0\n"
	"end\n\n"
	"function main(x, y, frame)\n"
	"	return all_whites()\n"
	"end";

static const char* entry = "main";

DOT_NS_BEGIN

controller::controller(hardware& hardware)
{
	_script.resize(buffer_length);
	memcpy(&_script[0], &defaultScript[0], buffer_length);

	_pixels.resize(hardware.rows * hardware.colums);
	_hardware = hardware;

	_interpreter.run_from_memory(&_script[0]);
}

void controller::execute(bool reload)
{
	if (reload)
	{
		_interpreter.run_from_memory(&_script[0]);
	}

	for (int j = 0; j < _hardware.colums; ++j)
	{
		for (int i = 0; i < _hardware.rows; ++i)
		{
			int value = _interpreter.call<int>(entry, i, j, _frame_count);
			int idx = i + _hardware.rows * j;
			_pixels[idx] = value;
		}
	}

	_frame_count++;
}

void controller::from_file(const std::string& script)
{
	std::ifstream script_file(script);
	if (script_file)
	{
		memset(&_script[0], 0, buffer_length);
		script_file.seekg(0, std::ios::end);
		std::streampos length = script_file.tellg();
		script_file.seekg(0, std::ios::beg);
		script_file.read(&_script[0], length);
	}

	execute(true);
}

void controller::get_script(std::vector<char>& buffer)
{
	buffer.resize(_script.size());
	memcpy(&buffer[0], &_script[0], _script.size());
}

void controller::set_script(std::vector<char>& buffer)
{
	_script.resize(_script.size());
	memcpy(&_script[0], &buffer[0], buffer.size());
}

void controller::send_to_hardware()
{
	if (!_serial.isOpen())
	{
		_serial.setPort(_hardware.port);
		_serial.setBaudrate(9600);
		_serial.setTimeout(serial::Timeout::simpleTimeout(1000));
	}

	/*int panel_width = 7;
	int num_panels = pixel_rows / panel_width;

	for (int p = 0; p < num_panels; ++p)
	{
		int panel = p;

		std::bitset<8> panelmask;
		panelmask[(num_panels - 1) - panel] = 1;

		std::vector<unsigned char> msg;
		msg.push_back(0x80);
		msg.push_back(0x83);
		msg.push_back((unsigned char)panelmask.to_ulong());

		for (uint32_t y = 0; y < pixel_columns; ++y)
		{
			std::bitset<8> bitmask;

			for (uint32_t x = 0; x < pixel_rows / 2; ++x)
			{
				int xx = x + (panel_width * panel);
				bitmask[x] = _display.get_pixel(xx, y);
			}

			unsigned long i = bitmask.to_ulong();
			msg.push_back((unsigned char)i);

		}

		msg.push_back(0x8F);
#ifndef _WIN32
		serial.write(&msg[0], msg.size());
#endif
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(50));*/
}

DOT_NS_END