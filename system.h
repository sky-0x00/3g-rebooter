#pragma once

#include "pch.h"

class system {
public:
	typedef unsigned timeout;
public:
	static bool reboot(_in timeout timeout = 0);
};