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
			0 /*1000.0 / (static_cast<unsigned(com::port::config::dcb_t::baud_rate::cbr_115200)> >> 3)*/, 1, 32, 0, 0
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
		static__at(_cp, "AT", result.data);
		if (!com::at::check(result.data).find(result.match))
			continue;
		assert(1 == result.match.size());

		// находим at-устройство производителя 'HUAWEI'...
		static__at(_cp, "AT+CGMI", result.data);
		if (!com::at::check(result.data).find(result.match))
			continue;
		assert(2 == result.match.size());
		if (find_info.names.vendor != string::to_upper(result.match.at(1)).string())
			continue;

		// ...и с именем 'E173'
		static__at(_cp, "AT+CGMM", result.data);
		if (!com::at::check(result.data).find(result.match))
			continue;
		assert(2 == result.match.size());
		if (find_info.names.model != string::to_upper(result.match.at(1)).string())
			continue;

		cp = std::move(_cp);
		return port;
	}
	return 0;
}


/*static*/ cstr_at device::static__at(
	_in const com::port &cp, _in cstr_at in, _out string_at &out
) {
	cp.send(in);
	return cp.recieve(out);
}
/*static*/ string_at device::static__at(
	_in const com::port &cp, _in cstr_at in
) {
	cp.send(in);
	return cp.recieve();
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


/*static*/ cstr_at device::static__check_for_data(
	_in const com::port &cp, _out string_at &data
) {
	return cp.recieve(data);
}
/*static*/ string_at device::static__check_for_data(
	_in const com::port &cp
) {
	return cp.recieve();
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

//------------------------------------------------------------------------------------------------------------------------------------------------------
device::sms::sms(
	_in const device &device
) :
	_device(device)
{}

bool device::sms::read_message(
	_in unsigned index, _out message &message
) const {
	com::at::result result;
	
	_device.at(("AT+CMGR=" + std::to_string(index)).c_str(), result.data);
	const auto is_matched = com::at::check(result.data).sms_read(result.match);
	assert(is_matched);
	
	// возможны 2 варианта: успешное чтение - 3 match'а и ошибка чтения (неверно указали индекс) - 1 match
	if (1 == result.match.size())
		return false;

	// match[0] - message::state
	// match[1] - message::content::size(), bytes
	// match[2] - message::content, hex string in upper-case
	
	message.state = static_cast<decltype(message.state)>(*result.match.at(0).first - '0');
	
	const auto &content = result.match.at(2);
	assert(0 == content.size() % 2);
	message.size_tpdu = content.size() >> 1;	
	message.pdu.reserve(message.size_tpdu);
	for (auto it = content.first; it < content.second; it += 2) {
		auto get_hbyte = [](_in char_at ch) -> unsigned {
			if (stdex::is__in_range__inclusive(ch, {'0', '9'}))
				return ch - '0';
			if (stdex::is__in_range__inclusive(ch, {'A', 'F'}))
				return ch + 10 - 'A';
			throw -1;
		};
		const unsigned pdu_octet = (get_hbyte(it[0]) << 4) | get_hbyte(it[1]);
		assert(pdu_octet < 0x100);
		message.pdu.push_back(pdu_octet);
	}
	assert(message.pdu.size() == message.size_tpdu);

	message.size_tpdu = std::strtoul(result.match.at(1).string().c_str(), nullptr, 10);
	return true;
}
device::sms::message::state_t device::sms::read_message(
	_in unsigned index, _out message::pdu_t &content
) const {
	message message;
	if (read_message(index, message)) {
		content.swap(message.pdu);
		return message.state;
	}
	trace(L"read_message(%i)", index);
	throw -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------
device::sms::info::info(
	_in const device &device
) :
	_device(device)
{}

device::sms::info::format device::sms::info::get_format(
) const {
	com::at::result result;

	_device.at("AT+CMGF?", result.data);
	const auto is_matched = com::at::check(result.data).sms_format(result.match);
	assert(is_matched);

	return static_cast<format>(*result.match.at(0).first - '0');
}

//------------------------------------------------------------------------------------------------------------------------------------------------------
