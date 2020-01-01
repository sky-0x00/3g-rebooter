#include "pch.h"
#include "device.h"
#include <regex>

com::port::number device::find(
	_in const std::vector<com::port::number> &numbers /*= com::port::static__enum()*/
) {
	return static__find(numbers, _cp);
}

com::port::number /*static*/ device::static__find(
	_in const std::vector<com::port::number> &numbers, _out com::port &cp
) {
	com::port::config config{ {
			com::port::config::dcb_t::baud_rate::cbr_115200,
			com::port::config::dcb_t::byte_size::bs_8,
			com::port::config::dcb_t::parity::no,
			com::port::config::dcb_t::stop_bits::one
		}, {
			0 /*1000.0 / (static_cast<unsigned(com::port::config::dcb_t::baud_rate::cbr_115200)> >> 3)*/, 1, 2, 0, 0
		} /*, 4096*/
	};

	for (const auto number : numbers) {
		
		com::port _cp;
		if (!_cp.open(number, config))
			continue;

		// находим at-устройство...
		if ("OK" != static__at(_cp, "AT"))
			continue;

		// находим at-устройство с именем 'e173'...

		cp = std::move(_cp);
		return number;
	}
	return 0;
}

cstr_at /*static*/ device::static__at(
	_in const com::port &cp, _in cstr_at in, _out string_at &out
) {
	cp.send(in);
	const auto result = cp.recieve(out);

	std::smatch sm;
	if (!std::regex_match(out, sm, std::regex(R"((.*)\n(.+)\n)")))
		return result;

	const auto n = sm.size();
	out = sm[2];
	return out.c_str();

}
string_at /*static*/ device::static__at(
	_in const com::port &cp, _in cstr_at in
) {
	string_at out;
	static__at(cp, in, out);
	return out;
}

cstr_at device::at(
	_in cstr_at in, _out string_at &out
) const {
	return static__at(_cp, in, out);
}
string_at device::at(
	_in cstr_at in
) const {
	return static__at(_cp, in);
}