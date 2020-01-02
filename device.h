#pragma once

#include "utils.h"

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

protected:
	com::port::number static static__find(_in const find_info &find_info, _out com::port &cp);		// 0 on not-found
	
	void static static__at(_in const com::port &cp, _in cstr_at in, _out com::at::result &out);
	com::at::result static static__at(_in const com::port &cp, _in cstr_at in);
	void at(_in cstr_at in, _out com::at::result &out) const;
	com::at::result at(_in cstr_at in) const;

private:
	com::port _cp;
};