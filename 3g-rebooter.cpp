// 3g-rebooter.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include "pch.h"
#include "application.h"
#include "device.h"
#include <iostream>
#include <cassert>
#include <windows.h>
#include "system.h"
#include <map>

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
	application application {class application::config {argc, argv}};
	if (!windows::reboot::privilege::static__set()) {
		const auto ExitCode = Winapi::GetLastError();
		std::cout << " win32-error: " << ExitCode << ", exiting..." << std::endl;
		return ExitCode;
	}
	if (application.config.close_some_precosses.is_close)
		application.close_some_processes(application.config.close_some_precosses.secondary_importance);
	std::cout << " ok" << std::endl;

	device::find_info device_fi {
		{"HUAWEI", "E173"}, {L"HUAWEI Mobile Connect - 3G PC UI Interface"}
	};
	if (0 == application.config.poling.comport_n)
		device_fi.ports = com::port::static__enum();
	else
		device_fi.ports.push_back(application.config.poling.comport_n);

	std::cout << "searching device \"" << device_fi.com.vendor << " " << device_fi.com.model << "\"...";
	const auto cpn = application.device.find(device_fi);
	if (0 == cpn) {
		std::cout << " error, not-found" << std::endl;
		return -1;
	}	
	std::cout << " ok, com(" << cpn << ")" << std::endl;

	// ...........
	if (device::sms::info::format::pdu != device::sms::info(application.device).get_format()) {
		trace(L"sms-info::format not pdu-type");
		return -1;
	}
	// ...........

	auto IsOk = application.set_ccf(application::ccf);
	assert(IsOk);

	IsOk = application.wt.set(application.config.poling.timeout_s);
	assert(IsOk);

	std::cout << "polling, start... timeout " << application.config.poling.timeout_s << " sec(s)" << std::endl << "polling, wait... #";
	
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
		
		com::at::result result;
//#ifdef _DEBUG
//		result.data = "\n+CMTI: \"SM\",6\n";
//#else
		result.data = application.device.check_for_data();
		if (result.data.empty()) {
			console::cursor_position::set(ccp);
			continue;
		}
//#endif
		// some data from com-port arrived, parse 'data' for list of non-empty strings 
		std::cout << "; new data, " << result.data.size() << " char(s):" << std::endl << result.data << std::endl << "checking, new-sms...";
		
		if (!com::at::check(result.data).new_sms(result.match))
			std::cout << " no";
		else {
			const auto &index = result.match.at(1).string();
			std::cout << " yes (\"" << result.match.at(0).string() << "\"," << index << ")" << std::endl << "reading sms...";
			// additional checks...
			device::sms::message sms_msg;
			IsOk = device::sms(application.device).read_message( std::strtoul(index.c_str(), nullptr, 10), sms_msg );
			assert(IsOk);
			std::cout << " ok, checking...";

			pdu::decoded pdu_d;	const pdu::encoded &pdu_e = sms_msg.pdu;
			if (pdu::decode(pdu_e, sms_msg.size_tpdu, pdu_d)) {
				windows::reboot::config wrc;
				if (windows::reboot::static__check_sms(pdu_d.message, wrc)) {
					const std::map<enum class windows::reboot::config::action, cstr_at> map {
						{ windows::reboot::config::action::reboot,   "reboot"   },
						{ windows::reboot::config::action::shutdown, "shutdown" },
					};
					std::cout << " ok, cmd-sms" << std::endl << "system, action: " << map.at(wrc.action);
					if (0 < wrc.timeout)
						std::cout << ", " << wrc.timeout << " sec(s)";
					std::cout << "...";
					if (windows::reboot::static__do(wrc))
						std::cout << " ok";								// можно и не выходить, все равно система будет перезагружена
					else
						std::cout << " win32-error: " << Winapi::GetLastError();
				} else {
					std::cout << " ok, not cmd-sms";
				}
			} else {
				std::cout << " error, decoding failed (not supported dcs?)";
			}
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
