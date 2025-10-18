#pragma once

#include "common.h"

DOT_NS_BEGIN

class window;

class view
{
public:
	virtual void initialize(window& w) { }
	virtual void on_render(window& w) { }
};

DOT_NS_END