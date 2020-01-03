#include "pch.h"
#include "application.h"
#include <windows.h>
#include <cassert>

//------------------------------------------------------------------------------------------------------------------------------------------------------
/*static*/ handle_t application::event_exit = nullptr;

application::application(
	_in const config &config /*= {}*/
) :
	_config(config), _ccf(nullptr)
{}

application::~application(
) {
	if (_ccf)
		set_ccf(nullptr);
}

/*static*/ application::console_control::function application::get_ccf(
) noexcept {
	return _ccf;
}

static BOOL WINAPI ConsoleControlFunction(
	_in DWORD CtrlType
) {
	return application::ccf(static_cast<application::console_control::event>(CtrlType)) ? TRUE : FALSE;
}

/*static*/ bool application::set_ccf(
	_in console_control::function ccf
) {
	auto IsOk = true;
	if (ccf) {		// нужно установить новый обработчик
		assert(ccf == application::ccf);
		if (!_ccf)
			IsOk = FALSE != Winapi::SetConsoleCtrlHandler(ConsoleControlFunction, TRUE);
	}
	else {			// нужно удалить текущий обработчик		
		if (_ccf)
			IsOk = FALSE != Winapi::SetConsoleCtrlHandler(ConsoleControlFunction, FALSE);
	}
	if (IsOk) {
		_ccf = ccf;
		if (ccf) {
			event_exit = Winapi::CreateEventW(NULL, FALSE, FALSE, NULL);
			assert(event_exit);
		}
		else {
			if (FALSE != Winapi::CloseHandle(event_exit))
				event_exit = nullptr;
			assert(!event_exit);
		}
	}
	return IsOk;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------
application::config::config(
	_in bool is_service, _in const poling_t &poling /*= {10}*/
) :
	is_service(is_service), poling(poling)
{}
application::config::config(
	_in const config &config
) :
	is_service(config.is_service), poling(config.poling)
{}

/*explicit*/ application::config::config(
	_in argc_t argc, _in const argv_t &argv
) :
	config(static__get(argc, argv))
{}

bool /*static*/ application::config::static__get(
	_in argc_t /*argc*/, _in const argv_t &/*argv*/, _out config &config
) noexcept {
	config = { false, {1} };		// temporary, need parser's implementation
	return true;
}
application::config /*static*/ application::config::static__get(
	_in argc_t argc, _in const argv_t &argv
) {
	config config {false};
	if (static__get(argc, argv, config))
		return config;
	trace(L"static__get(): false");
	throw -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------
application::waitable_timer::waitable_timer(
) :
	handle(Winapi::CreateWaitableTimerW(NULL, FALSE, NULL))
{
	assert(handle);
}
application::waitable_timer::~waitable_timer(
) {
	Winapi::CancelWaitableTimer(handle);
}

bool application::waitable_timer::set(
	_in unsigned timeout /*sec*/
) const {
	const LARGE_INTEGER DueTime { /*-10000000 * timeout*/ /*-1*/ 0 };
	return FALSE != Winapi::SetWaitableTimer(handle, &DueTime, 1000*timeout, NULL, NULL, FALSE);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------