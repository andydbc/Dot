#pragma once

#include "common.h"
#include "window.h"

DOT_NS_BEGIN

class view
{
public:
	virtual void initialize() { }
	virtual void on_render(window& w) { }
};

DOT_NS_END