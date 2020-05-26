#pragma once 

#include "common.h"
#include "interpreter.h"

#include "RtMidi.h"
#include <serial/serial.h>

#include <string>
#include <vector>

DOT_NS_BEGIN

struct hardware
{
	int rows;
	int colums;
	std::string port;
};

class controller
{
public:

	controller();
	controller(hardware& description);

	void execute(bool reload = false);

	void open_midi();
	void set_hardware(const hardware& h);
	
	void from_file(const std::string& script);

	void get_script(std::vector<char>& buffer);
	void set_script(std::vector<char>& buffer);

	const std::vector<bool>& get_pixels() const
	{
		return _pixels;
	}

	const std::string& get_script_path() { return _script_path; }
	void save_script();

	hardware& get_hardware() { return _hardware; }
	void send_to_hardware();

private:
	
	hardware _hardware;
	interpreter _interpreter;
	std::vector<char> _script;
	std::vector<bool> _pixels;
	std::string _script_path;
	int _frame_count;
	bool _exec_sucess;

	serial::Serial _serial;
	RtMidiIn _midi;
};

DOT_NS_END