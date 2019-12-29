#pragma once

#include "pch.h"

class application {
public:
	struct config {
	};
public:
	application(_in const config &config = {});
private:
	const config _config;
};