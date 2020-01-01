#pragma once

#include "pch.h"
#include "device.h"

class application {
public:
	class config {
	public:
		bool is_service;
		struct poling_t {
			unsigned timeout/*sec*/;
		};
		poling_t poling;

		config(_in bool is_service, _in const poling_t &poling = {10});
		config(_in const config &config);
		explicit config(_in argc_t argc, _in const argv_t &argv);

	protected:
		bool static static__get(_in argc_t argc, _in const argv_t &argv, _out config &config) noexcept;
		config static static__get(_in argc_t argc, _in const argv_t &argv);
	};
public:
	application(_in const config &config);
	device device;
private:
	const config _config;
};