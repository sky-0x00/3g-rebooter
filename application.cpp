#include "pch.h"
#include "application.h"
#include <windows.h>
#include <tlhelp32.h>
#include <cassert>
#include <algorithm>
#include <mutex>
#include <array>

//------------------------------------------------------------------------------------------------------------------------------------------------------
/*static*/ handle_t application::event_exit = nullptr;

application::application(
	_in const class config &config /*= {}*/
) :
	config(config), _ccf(nullptr)
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

struct defs {
	struct image {
		cstr_t path;
		typedef std::initializer_list<image> list;
	};
	struct importance {
		image::list firstly, secondary;
	};
	static const importance static__importance;

	struct image_ex {
		string_t path;
		cstr_t name;
		template <unsigned size> using list = std::array<image_ex, size>;
	};
	template <unsigned size_firstly, unsigned size_secondary> struct importance_ex {
		image_ex::list<size_firstly> firstly;
		image_ex::list<size_secondary> secondary;
	};
	static importance_ex</*importance::firstly.size()*/2, /*importance::secondary.size()*/2> static__importance_ex;						// size as size of 'static__importance'
};

/*static*/ const defs::importance defs::static__importance {
	{
		{LR"(%ProgramFiles(x86)%\MegaFon Internet\MegaFon Internet.exe)"},
		{LR"(%ProgramFiles(x86)%\Mobile Partner\Mobile Partner.exe)"}
	},
	{
		{LR"(%ProgramData%\DatacardService\Temp\Mobile Partner\Setup.exe)"},
		{LR"(%ProgramFiles(x86)%\MegaFon Internet\UpdateDog\ouc.exe)"}
	}
};
/*static*/ defs::importance_ex</*importance::firstly.size()*/2, /*importance::secondary.size()*/2> defs::static__importance_ex;			// size as size of 'static__importance'

struct pid {
	typedef unsigned value_type;
	value_type value;
	template <unsigned size> using list = std::array<pid, size>;
};

/*static*/ void application::close_some_processes(
	_in bool is_close__secondary_importance
) {

	auto static__importance_ex = []() {
		
		auto get_ex__path = [](
			_in cstr_t image_path
		) -> string_t {

			struct /*buffer*/ {
				char_t data[512]; 
				unsigned size;
			} buffer;
			buffer.size = Winapi::ExpandEnvironmentStringsW(image_path, buffer.data, _countof(buffer.data));
			assert(stdex::is__in_range(buffer.size, {1U, static_cast<unsigned>(_countof(buffer.data))}));

			return {buffer.data, buffer.size};
		};
		auto get_ex__name = [](_in const string_t &path) {
			auto pos = path.rfind(L'\\');
			return ++pos + path.c_str();
		};

		unsigned i = 0;
		for (const auto &image : defs::static__importance.firstly) {
			auto &image_ex = defs::static__importance_ex.firstly[i];
			image_ex.path = get_ex__path(image.path);
			image_ex.name = get_ex__name(image_ex.path);
			++i;
		}
		i = 0;
		for (const auto &image : defs::static__importance.secondary) {
			auto &image_ex = defs::static__importance_ex.secondary[i];
			image_ex.path = get_ex__path(image.path);
			image_ex.name = get_ex__name(image_ex.path);
			++i;
		}
	};

	static std::once_flag of;
	std::call_once(of, static__importance_ex);

	const auto hSnapshot = Winapi::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot) {
		trace(L"CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS): E(0x%X)", Winapi::GetLastError());
		return;
	}
	std::unique_ptr<std::remove_pointer<decltype(hSnapshot)>::type, decltype(&Winapi::CloseHandle)> hs(hSnapshot, &Winapi::CloseHandle);

	struct importance {
		pid::list<defs::static__importance_ex.firstly.size()> firstly;
		pid::list<defs::static__importance_ex.secondary.size()> secondary;
	};
	struct importance importance {};

	struct check {
		static bool name(_in cstr_t lhs, _in cstr_t rhs) {
			return 0 == _wcsicmp(lhs, rhs);
		}
		static bool path(_in unsigned pid, _in cstr_t path) {

			const auto hSnapshot = Winapi::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
			if (INVALID_HANDLE_VALUE == hSnapshot) {
				const auto LastError = Winapi::GetLastError();
				trace(L"Winapi::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, %u): E(0x%X)", pid, LastError);
				throw LastError;
			}
			std::unique_ptr<std::remove_pointer<decltype(hSnapshot)>::type, decltype(&Winapi::CloseHandle)> hs(hSnapshot, &Winapi::CloseHandle);

			MODULEENTRY32W me {sizeof(me)};
			if (FALSE == Winapi::Module32FirstW(hSnapshot, &me)) {
				const auto LastError = Winapi::GetLastError();
				trace(L"Winapi::Module32FirstW(%s): E(0x%X)", hSnapshot, LastError);
				throw LastError;
			}

			// first module is image (.exe) module
			return name(path, me.szExePath);
		}
	};
	
	{
		PROCESSENTRY32W pe {sizeof(pe)};
		for (auto IsContinue = FALSE != Winapi::Process32FirstW(hSnapshot, &pe); IsContinue; IsContinue = Winapi::Process32NextW(hSnapshot, &pe)) {

			if (0 == pe.th32ProcessID)
				continue;

			{
				const auto it = std::make_pair(defs::static__importance_ex.firstly.begin(), defs::static__importance_ex.firstly.end());
				const auto it_found = std::find_if(
					it.first, it.second, [&pe](
						_in const defs::image_ex &image_ex
					) {
						return check::name(image_ex.name, pe.szExeFile);
					}
				);
				if (it_found != it.second) {

					if (!check::path(pe.th32ProcessID, it_found->path.c_str()))
						continue;

					const auto i = std::distance(it.first, it_found);
					importance.firstly[i].value = pe.th32ProcessID;
					continue;
				}
			}

			if (!is_close__secondary_importance)
				continue;

			{
				const auto it = std::make_pair(defs::static__importance_ex.secondary.begin(), defs::static__importance_ex.secondary.end());
				const auto it_found = std::find_if(
					it.first, it.second, [&pe](
						_in const defs::image_ex &image_ex
					) {
						return 0 == _wcsicmp(image_ex.name, pe.szExeFile);
					}
				);
				if (it_found != it.second) {
					
					if (!check::path(pe.th32ProcessID, it_found->path.c_str()))
						continue;

					const auto i = std::distance(it.first, it_found);
					importance.secondary[i].value = pe.th32ProcessID;
					continue;
				}
			}

		}
		assert(ERROR_NO_MORE_FILES == Winapi::GetLastError());
	}

	for (const auto &pid : importance.firstly)
		if (0 != pid.value)
			close_process(pid.value);

	if (!is_close__secondary_importance)
		return;

	for (const auto &pid : importance.secondary)
		if (0 != pid.value)
			close_process(pid.value);
}

void application::close_some_processes(
) const {
	close_some_processes(config.close_some_precosses.secondary_importance);
}

/*static*/ set_lasterror(bool) application::close_process(
	_in unsigned pid, _in unsigned exit_code /*= ERROR_PROCESS_ABORTED(1067L)*/
) {
	auto hProcess = Winapi::OpenProcess(PROCESS_TERMINATE, FALSE, pid);
	if (!hProcess)
		return false;
	std::unique_ptr<std::remove_pointer<decltype(hProcess)>::type, decltype(Winapi&::CloseHandle)> ph(hProcess, Winapi&::CloseHandle);

	return FALSE != Winapi::TerminateProcess(hProcess, exit_code);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------
application::config::config(
	_in bool is__start_as_service, 
	_in const struct close_some_precosses &close_some_precosses,
	_in const struct poling &poling, 
	_in const struct sms &sms
) :
	is__start_as_service(is__start_as_service), 
	close_some_precosses(close_some_precosses), 
	poling(poling), 
	sms(sms)
{}
application::config::config(
	_in const config &config
) :
	config(
		config.is__start_as_service, 
		config.close_some_precosses,
		config.poling, 
		config.sms
	)
{}

/*explicit*/ application::config::config(
	_in argc_t argc, _in const argv_t &argv
) :
	config(static__get(argc, argv))
{}

/*static*/ bool application::config::static__get(
	_in argc_t /*argc*/, _in const argv_t &/*argv*/, _out config &config
) noexcept {
	// temporary, need parser's implementation
	config = { 
		/*is__start_as_service*/	false,
		/*close_some_precosses*/	{true, true},
		/*poling*/					{0, 4},
		/*sms*/						{false}
	};
	return true;
}
/*static*/ class application::config application::config::static__get(
	_in argc_t argc, _in const argv_t &argv
) {
	config config;
	if (static__get(argc, argv, config))
		return config;
	trace(L"static__get(): false");
	throw ERROR_BAD_ARGUMENTS;
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