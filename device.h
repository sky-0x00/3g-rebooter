#pragma once

#include "utils.h"
#include <optional>

class device {
public:
	struct find_info {

		struct names_t {
			string_at vendor, model;
		};
		names_t names;

		typedef std::vector<com::port::number> ports_t;
		ports_t ports;

		find_info(_in const names_t &names, _in const ports_t &ports = { com::port::static__enum() });
	};
	com::port::number find(_in const find_info &find_info);											// 0 on not-found

	struct sms_info {
		// type						// AT+CSMS=<V>, AT+CSMS?, AT+CSMS=?
		enum class format {			// AT+CMGF=<V>, AT+CMGF?, AT+CMGF=?
			pdu = 0, text = 1
		};
		format format;
	};
	const sms_info& sms_info();

	cstr_at check_for_data(_out string_at &data) const;
	string_at check_for_data() const;

protected:
	static com::port::number static__find(_in const find_info &find_info, _out com::port &cp);		// 0 on not-found
	
	static void static__at(_in const com::port &cp, _in cstr_at in, _out com::at::result &out);
	//com::at::result static static__at(_in const com::port &cp, _in cstr_at in);
	void at(_in cstr_at in, _out com::at::result &out) const;
	//com::at::result at(_in cstr_at in) const;

	static cstr_at static__check_for_data(_in const com::port &cp, _out string_at &data);
	static string_at static__check_for_data(_in const com::port &cp);

private:
	std::optional<struct sms_info> _sms_info;
	com::port _cp;
};