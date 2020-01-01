#pragma once

#include "utils.h"

class device {
public:
	com::port::number find(_in const std::vector<com::port::number> &numbers = com::port::static__enum());				// 0 on not-found
protected:
	com::port::number static static__find(_in const std::vector<com::port::number> &numbers, _out com::port &cp);		// 0 on not-found
	
	cstr_at static static__at(_in const com::port &cp, _in cstr_at in, _out string_at &out);
	string_at static static__at(_in const com::port &cp, _in cstr_at in);
	cstr_at at(_in cstr_at in, _out string_at &out) const;
	string_at at(_in cstr_at in) const;

private:
	com::port _cp;
};