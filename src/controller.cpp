#include "controller.h"

#include <algorithm>
#include <bitset>
#include <fstream>
#include <thread>

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

controller::controller()
{
	open_midi();
	_script.resize(buffer_length);
	memcpy(&_script[0], &defaultScript[0], buffer_length);

	std::string packages = "package.path = package.path .. \";resources/scripts/?.lua\"";
	_interpreter.run_from_memory(packages);
	_interpreter.run_from_memory(&_script[0]);

	_frame_count = 0;
}

controller::controller(hardware& hardware)
{
	open_midi();
	_pixels.resize(hardware.rows * hardware.colums);
	_hardware = hardware;
	
	_script.resize(buffer_length);
	memcpy(&_script[0], &defaultScript[0], buffer_length);

	std::string packages = "package.path = package.path .. \";resources/scripts/?.lua\"";
	_interpreter.run_from_memory(packages);
	_interpreter.run_from_memory(&_script[0]);

	_frame_count = 0;
}

void controller::open_midi()
{
	unsigned int ports_count = _midi.getPortCount();

	if (ports_count > 0)
		_midi.openPort(0);
}

void controller::execute(bool reload)
{
	if (reload)
	{
		_exec_sucess = _interpreter.run_from_memory(&_script[0]);
	}

	if (!_exec_sucess)
	{
		return;
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

	std::vector<unsigned char> msg;
	_midi.getMessage(&msg);
	int nBytes = msg.size();
	if(nBytes == 3)
	{
		_interpreter.call<void>("on_midi_input", msg[0], msg[1]);
	}
}

void controller::set_hardware(const hardware& h)
{
	_pixels.resize(h.rows * h.colums);
	_hardware = h;
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

		_script_path = script;
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

void controller::save_script()
{
	if (!_script_path.empty())
	{
		std::ofstream script;
		script.open(_script_path);
		auto it = std::find(_script.begin(), _script.end(), '\0');
		script.write(&_script[0], std::distance(_script.begin(), it));
		script.close();
	}
}

void controller::send()
{
	if (_hardware.port.empty())
		return;
	
	if (!_serial.isOpen())
	{
		_serial.setPort(_hardware.port);
		_serial.setBaudrate(9600);
		_serial.setTimeout(serial::Timeout::simpleTimeout(1000));
		_serial.open();
	}

	int panel_width = 7;
	int num_panels = _hardware.rows / panel_width;

	for (int p = 0; p < num_panels; ++p)
	{
		int panel = p;

		std::bitset<8> panelmask;
		panelmask[(num_panels - 1) - panel] = 1;

		std::vector<unsigned char> msg;
		msg.push_back(0x80);
		msg.push_back(0x83);
		msg.push_back((unsigned char)panelmask.to_ulong());

		for (int32_t y = 0; y < _hardware.colums; ++y)
		{
			std::bitset<8> bitmask;

			for(int i = 0; i < panel_width; ++i)
			{
				int idx = (i + _hardware.rows * y) + panel_width * p;
				bitmask[i] = _pixels[idx];
			}

			unsigned long b = bitmask.to_ulong();
			msg.push_back((unsigned char)b);
		}

		msg.push_back(0x8F);
		_serial.write(&msg[0], msg.size());
	}
}

DOT_NS_END