#include "pch.h"
#include "system.h"
#include <windows.h>
#include <regex>
#include <map>

/*static*/ bool windows::reboot::static__check_sms(
	_in _in const struct pdu::decoded::message &message, _out config &config
) {
	std::wsmatch sm;
//#ifdef _DEBUG
//	const string_t &message_text = L"sys-r 10 с сайта megafon.ru";
//#endif
	if (!std::regex_match(message.text, sm, std::wregex(LR"(sys-(r|s)(?: (\d+))?(?: .*)?)")))
		return false;
	assert(3 == sm.size());
	
	const std::map<char_t, enum class config::action> map{
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
//#ifdef _DEBUG
//	// ok
//	//return true;
//
//	// fail, not have privilage
//	Winapi::SetLastError(ERROR_ACCESS_DENIED);
//	return false;
//#else
	const std::map<enum class config::action, BOOL> reboot_after_shutdown {
		{ config::action::reboot,   TRUE  },
		{ config::action::shutdown, FALSE },
	};
	return FALSE != Winapi::InitiateSystemShutdownW(NULL, NULL, config.timeout, TRUE, reboot_after_shutdown.at(config.action));
//#endif
}

/*static*/ bool windows::reboot::static__check_privs(
) {
	// TODO
	return true;
}