#pragma once 

#include "common.h"
#include "interpreter.h"

// #include "RtMidi.h"
#include <serial/serial.h>

#include <string>
#include <vector>

DOT_NS_BEGIN

// struct hardware
// {
// 	int rows;
// 	int colums;
// 	std::string port;
// };

class controller
{
public:

	controller();
	controller(int width, int height);

	void execute(bool reload = false);

	// void open_midi();
	
	void from_file(const std::string& script);

	void get_script(std::vector<char>& buffer);
	void set_script(std::vector<char>& buffer);

	const std::vector<bool>& get_pixels() const { return _pixels; }
	const std::string& get_script_path() { return _script_path; }
	void save_script();
	interpreter& get_interpreter() { return _interpreter; }
	void send(serial::Serial& serial);

	int get_width() const { return _width; }
	int get_height() const { return _height; }
	int get_frame_count() const { return _frame_count; }
	
private:
	
	interpreter _interpreter;
	std::vector<char> _script;
	std::vector<bool> _pixels;
	std::string _script_path;
	int _frame_count;
	bool _exec_sucess;

	int _width;
	int _height;
};

DOT_NS_END