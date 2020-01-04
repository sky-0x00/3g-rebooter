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

		// считаем весь имеющийся буфер (могут скопиться мусорные данные)
		com::at::result result;
		static__check_for_data(_cp, result.data);

		// находим at-устройство		
		static__at(_cp, "AT", result);
		if ((2 != result.match.size()) || (com::at::result::ok() != result.match.at(1).string()))
			continue;

		// находим at-устройство производителя 'HUAWEI'...
		static__at(_cp, "AT+CGMI", result);
		if ((3 != result.match.size()) || (com::at::result::ok() != result.match.at(2).string()))
			continue;
		if (find_info.names.vendor != string::to_upper(result.match.at(1)).string())
			continue;

		// ...и с именем 'E173'
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
			out.match.emplace_back(it->first, it->second);
}
//com::at::result /*static*/ device::static__at(
//	_in const com::port &cp, _in cstr_at in
//) {
//	com::at::result out;
//	static__at(cp, in, out);
//	return out;
//}

void device::at(
	_in cstr_at in, _out com::at::result &out
) const {
	static__at(_cp, in, out);
}
//com::at::result device::at(
//	_in cstr_at in
//) const {
//	return static__at(_cp, in);
//}

const struct device::sms_info& device::sms_info(
) {
	if (!_sms_info.has_value()) {

		_sms_info.emplace();
		assert(_sms_info.has_value());

		com::at::result result;
		at("AT+CMGF?", result);
		assert((3 == result.match.size()) && (com::at::result::ok() == result.match.at(2).string()));
		
		const auto &format = result.match.at(1);
		std::smatch sm;
		const auto is_match = std::regex_match(format.first, format.second, sm, std::regex(R"(\+CMGF: (0|1))"));
		assert(is_match && (2 == sm.size()));
		_sms_info->format = static_cast<decltype(_sms_info->format)>(*sm[1].first - '0');
	}
	return _sms_info.value();
}

/*static*/ cstr_at device::static__check_for_data(
	_in const com::port &cp, _out string_at &data
) {
	return cp.recieve(data);
}
/*static*/ string_at device::static__check_for_data(
	_in const com::port &cp
) {
	string_at data;
	static__check_for_data(cp, data);
	return data;
}

cstr_at device::check_for_data(
	_out string_at &data
) const {
	return static__check_for_data(_cp, data);
}
string_at device::check_for_data(
) const {
	return static__check_for_data(_cp);
}