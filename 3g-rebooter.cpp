// 3g-rebooter.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include "pch.h"
#include "application.h"
#include "device.h"
#include <iostream>
#include <cassert>
#include <windows.h>

/*static*/ bool application::ccf(
	_in console_control::event /*cce*/
) {
	assert(event_exit);
	const auto IsOk = Winapi::SetEvent(event_exit);
	assert(IsOk);

	return true;
}

int wmain(
	_in argc_t argc, _in const argv_t &argv
) {
	std::cout << "application initialization...";
	application application {application::config {argc, argv}};
	std::cout << " ok" << std::endl;
	
	const device::find_info device_fi {
		{"HUAWEI", "E173"}
	};
	std::cout << "finding device \"" << device_fi.names.vendor << " " << device_fi.names.model << "\"...";
	const auto cpn = application.device.find(device_fi);
	if (0 == cpn) {
		std::cout << " error, not-found" << std::endl;
		return -1;
	}	
	std::cout << " ok, com(" << cpn << ")" << std::endl;

	// ...........
	if (device::sms_info::format::pdu != application.device.sms_info().format) {
		trace(L"sms_info::format not pdu-type");
		return -1;
	}

	auto IsOk = application.set_ccf(application::ccf);
	assert(IsOk);

	IsOk = application.wt.set(application._config.poling.timeout_s);
	assert(IsOk);

	std::cout << "polling, start... timeout (" << application._config.poling.timeout_s << ")s" << std::endl << "polling, wait... #";
	
	auto ccp = console::cursor_position::get();
	const handle_t handles[] {application.wt.handle, application::event_exit};

	for (unsigned i = 0; ;) {

		std::cout << ++i;

		switch (Winapi::WaitForMultipleObjects(_countof(handles), handles, FALSE, INFINITE)) {
		case Winapi::WaitObject(0):
			break;
		case Winapi::WaitObject(1):
			std::cout << ", exit..." << std::endl;
			//trace(L"WaitForSingleObject(): EventExit");
			return ERROR_CANCELLED;
		default:
			trace(L"WaitForSingleObject(): 0x%X", Winapi::GetLastError());
			return Winapi::GetLastError();
		}
		
		/*const */string_at /*&*/data = application.device.check_for_data();
		if (data.empty()) {
			console::cursor_position::set(ccp);
			continue;
		}

		data = "\n+CMTI: \"SM\",2\n";
		// some data from com-port arrived, parse 'data' for list of non-empty strings 
		std::cout << "; new data, " << data.size() << " char(s):" << std::endl << data << std::endl << "checking, new-sms...";

		const auto &new_sms = com::at::check(data).new_sms();
		if (new_sms.empty())
			std::cout << " no";
		else {
			assert(2 == new_sms.size());
			std::cout << " yes (" << new_sms.at(0).string() << "," << new_sms.at(1).string() << ")" << std::endl << "reading sms...";
			// additional checks...
		}

		std::cout << std::endl << "polling, wait... #";
		ccp = console::cursor_position::get();
	}

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
