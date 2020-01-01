#pragma once

#include "pch.h"

class application {
public:
	struct config {
		bool is_service = false;
		struct poling {
			unsigned timeout /*sec*/;
		};
		struct poling poling {10};
	};
public:
	application(_in const config &config = {});
private:
	const config _config;
};