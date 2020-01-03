#include "pch.h"
#include "device.h"
#include <regex>
#include <cassert>

device::find_info::find_info(
	_in const names_t &names, _in const ports_t &ports /*= { com::port::static__enum() }*/
) :
	names(names), ports(ports)
{
	string::to_upper(this->names.vendor);
	string::to_upper(this->names.model);
}

com::port::number device::find(
	_in const find_info &find_info
) {
	return static__find(find_info, _cp);
}

com::port::number /*static*/ device::static__find(
	_in const find_info &find_info, _out com::port &cp
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

	for (const auto port : find_info.ports) {
		
		com::port _cp;
		if (!_cp.open(port, config))
			continue;

		com::at::result result;

		// ������� ���� ��������� ����� (����� ��������� �������� ������)
		_cp.recieve(result.data);

		// ������� at-����������		
		static__at(_cp, "AT", result);
		if ((2 != result.match.size()) || (com::at::result::ok() != result.match.at(1).string()))
			continue;

		// ������� at-���������� ������������� 'HUAWEI'...
		static__at(_cp, "AT+CGMI", result);
		if ((3 != result.match.size()) || (com::at::result::ok() != result.match.at(2).string()))
			continue;
		if (find_info.names.vendor != string::to_upper(result.match.at(1)).string())
			continue;

		// ...� � ������ 'E173'
		static__at(_cp, "AT+CGMM", result);
		if ((3 != result.match.size()) || (com::at::result::ok() != result.match.at(2).string()))
			continue;
		if (find_info.names.model != string::to_upper(result.match.at(1)).string())
			continue;

		cp = std::move(_cp);
		return port;
	}
	return 0;
}

void /*static*/ device::static__at(
	_in const com::port &cp, _in cstr_at in, _out com::at::result &out
) {
	out.clear();		// call only 'out.match.clear()' will be enough

	cp.send(in);
	cp.recieve(out.data);

	std::smatch sm;
	if (!std::regex_match(out.data, sm, std::regex(R"((.*)\n(?:(.+)\n\n)?(.+)\n)")))
		return;

	out.match.reserve(sm.size() - 1);
	for (auto it = sm.cbegin(); ++it != sm.cend(); )
		if (it->matched)
			out.match.push_back({ { it->first, it->second } });
}
com::at::result /*static*/ device::static__at(
	_in const com::port &cp, _in cstr_at in
) {
	com::at::result out;
	static__at(cp, in, out);
	return out;
}

void device::at(
	_in cstr_at in, _out com::at::result &out
) const {
	static__at(_cp, in, out);
}
com::at::result device::at(
	_in cstr_at in
) const {
	return static__at(_cp, in);
}