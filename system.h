#pragma once

#include "pch.h"

namespace windows {

	class reboot {
	public:
		//typedef unsigned timeout;
	public:
		static set_lasterror(bool) static__do(_in unsigned timeout = 0);
		static bool static__check_privs();
		//static bool static__register_application_for_restart();
	};

}	// namespace system