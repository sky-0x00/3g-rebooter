#include "pch.h"
#include "system.h"
#include <windows.h>
#include <regex>
#include <map>

//------------------------------------------------------------------------------------------------------------------------------------------------------
/*static*/ bool windows::reboot::static__check_sms(
	_in const struct pdu::decoded::message &message, _out config &config
) {
	std::wsmatch sm;
//#ifdef _DEBUG
//	const string_t &message_text = L"sys-r 10 с сайта megafon.ru";
//#endif
	if (!std::regex_match(message.text, sm, std::wregex(LR"(sys-(r|s)(?: (\d+))?(?: .*)?)")))
		return false;
	assert(3 == sm.size());
	
	const std::map<char_t, enum class config::action> map {
		{ L'r', config::action::reboot   },
		{ L's', config::action::shutdown }
	};
	config.action = map.at(*sm[1].first);
	config.timeout = sm[2].matched ? std::wcstoul(&*sm[2].first, nullptr, 10) : 0;
	
	return true;
}

/*static*/ set_lasterror(bool) windows::reboot::static__do(
	_in const config &config
) {
#ifdef _DEBUG
	// ok
	return true;

	// fail, not have privilage
	Winapi::SetLastError(ERROR_ACCESS_DENIED);
	return false;
#else
	const std::map<enum class config::action, BOOL> reboot_after_shutdown {
		{ config::action::reboot,   TRUE  },
		{ config::action::shutdown, FALSE },
	};
	return FALSE != Winapi::InitiateSystemShutdownW(NULL, NULL, config.timeout, TRUE, reboot_after_shutdown.at(config.action));
#endif
}

//------------------------------------------------------------------------------------------------------------------------------------------------------
/*static*/ set_lasterror(bool) windows::reboot::privilege::static__get(
	_out bool &is_enabled
) noexcept {	
	HANDLE hToken;
	if (FALSE == Winapi::OpenProcessToken(Winapi::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken))
		return false;
	std::unique_ptr<std::remove_pointer<decltype(hToken)>::type, decltype(&Winapi::CloseHandle)> token(hToken, &Winapi::CloseHandle);

	//TODO(...)
	//TOKEN_PRIVILEGES *tkp = nullptr;
	//DWORD tkp_size = 0;
	//if (FALSE == Winapi::GetTokenInformation(hToken, TokenPrivileges, tkp, tkp_size, &tkp_size))
	//	return false;
	////...

	return true;
}
/*static*/ bool windows::reboot::privilege::static__get(
) {
	bool is_enabled;
	if (static__get(is_enabled))
		return is_enabled;
	throw Winapi::GetLastError();
}

/*static*/ set_lasterror(bool) windows::reboot::privilege::static__set(
) noexcept {
	HANDLE hToken;
	if (FALSE == Winapi::OpenProcessToken(Winapi::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken))
		return false;
	std::unique_ptr<std::remove_pointer<decltype(hToken)>::type, decltype(&Winapi::CloseHandle)> token(hToken, &Winapi::CloseHandle);

	TOKEN_PRIVILEGES tkp {1, LUID_AND_ATTRIBUTES{{}, SE_PRIVILEGE_ENABLED}};
	if (FALSE == Winapi::LookupPrivilegeValueW(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid))
		return false;

	if (FALSE == Winapi::AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, NULL))
		return false;

	return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------
