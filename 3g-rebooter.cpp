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

	auto IsOk = application.set_ccf(application::ccf);
	assert(IsOk);

	IsOk = application.wt.set(application._config.poling.timeout);
	assert(IsOk);

	std::cout << "polling, start... timeout (" << application._config.poling.timeout << ")s" << std::endl << "polling, wait... #";
	COORD CursorPosition;
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		IsOk = FALSE != Winapi::GetConsoleScreenBufferInfo(Winapi::GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		assert(IsOk);
		CursorPosition = csbi.dwCursorPosition;
	}
	const handle_t handles[] {application.wt.handle, application::event_exit};
	for (unsigned i = 0; ;) {
		std::cout << ++i;
		const auto WaitObject = Winapi::WaitForMultipleObjects(_countof(handles), handles, FALSE, INFINITE);
		switch (WaitObject) {
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
		// ...
		// std::cout << " ok" << std::endl;
		{
			IsOk = FALSE != Winapi::SetConsoleCursorPosition(Winapi::GetStdHandle(STD_OUTPUT_HANDLE), CursorPosition);
			assert(IsOk);
		}
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
