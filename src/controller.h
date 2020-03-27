#pragma once 

#include "common.h"
#include "interpreter.h"

#include <memory>
#include <string>
#include <vector>

DOT_NS_BEGIN

struct hardware
{
	int rows;
	int colums;
};

class controller
{
public:

	controller(hardware& description);

	void execute(bool reload = false);

	void from_file(const std::string& script);

	void get_script(std::vector<char>& buffer);
	void set_script(std::vector<char>& buffer);

	hardware& get_hardware() {
		return _hardware;
	}

	const std::vector<bool> get_pixels() const {
		return _pixels;
	}

	void send_to_hardware();

private:
	interpreter _interpreter;
	std::vector<char> _script;
	std::vector<bool> _pixels;
	hardware _hardware;
	int _frame_count;
};

DOT_NS_END