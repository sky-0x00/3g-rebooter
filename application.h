#pragma once

#include "pch.h"
#include "device.h"
#include <atomic>

class application {
public:

	struct console_control {
		enum class event {
			ctrl_c = 0,
			ctrl_break = 1
		};
		typedef bool(*function)(_in event cce);
	};
	struct waitable_timer {
		//typedef bool(*function)(_in event /*event*/);

		waitable_timer();
		~waitable_timer();
		bool set(_in unsigned timeout /*sec*/) const;

		const handle_t handle;
	};

	class config {
	public:
		bool is__start_as_service;

		struct poling {
			com::port::number comport_n;				// ���� 0, �� ����� ���������, ����� ����� �������� ������ ����� �� ��������� ����
			unsigned timeout_s /*sec*/;					// ��� - 1 ���
		};
		struct poling poling;

		struct sms {
			bool is__remove_after_processed;			// TODO
		};
		struct sms sms;

		config(_in bool is__start_as_service, _in const struct poling &poling, _in const struct sms &sms);
		config(_in const config &config);
		explicit config(_in argc_t argc, _in const argv_t &argv);

	protected:
		config() = default;

		bool static static__get(_in argc_t argc, _in const argv_t &argv, _out config &config) noexcept;
		config static static__get(_in argc_t argc, _in const argv_t &argv);
	};

public:
	application(_in const config &config);
	~application();

	console_control::function get_ccf() noexcept;
	bool set_ccf(_in console_control::function function);

public:
	static bool ccf(_in console_control::event cce);				// in main ('3g-rebooter') module

public:
	device device;
	waitable_timer wt;
	const config config;
	static handle_t event_exit;

private:
	console_control::function _ccf;
	
};