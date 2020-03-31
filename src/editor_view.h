#pragma once

#include "dot.h"

class editor_view : public dot::view
{
public:
	editor_view(dot::controller* c) : _controller(c) { }

	void initialize(dot::window& w) override;
	void on_render(dot::window& w) override;

	void on_save();
private:
	bool _has_changes;
	std::vector<char> _buffer;
	dot::controller* _controller;
};