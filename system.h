#pragma once

#include "pch.h"
#include "utils.h"

namespace windows {

	class reboot {
	public:
		//typedef unsigned timeout;
		struct config {
			enum class action {
				reboot, shutdown
			};
			action action;
			unsigned timeout;
		};
	public:
		class privilege {
		public:
			static set_lasterror(bool) static__set() noexcept;
		protected:
			static set_lasterror(bool) static__get(_out bool &is_enabled) noexcept;
			static bool static__get();
		};
	public:
		static bool static__check_sms(_in const struct pdu::decoded::message &message, _out config &config);
		static set_lasterror(bool) static__do(_in const config &config);
		//static bool static__register_application_for_restart();
	};

}	// namespace system