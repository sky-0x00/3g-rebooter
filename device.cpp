#include "pch.h"
#include "device.h"
#include <regex>
#include <cassert>
#include <windows.h>
#include "application.h"

device::find_info::find_info(
	_in const com_t &com, _in const usb_t &usb, _in const ports_t &ports /*= {}*/
) :
	com(com), usb(usb), ports(ports)
{
	string::to_lower(this->com.vendor);
	string::to_lower(this->com.model);
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
		if (!_cp.open(port, config)) {
			if (Winapi::GetLastError() != ERROR_BUSY)
				continue;
			application::close_some_processes(false);
			if (!_cp.open(port, config))
				continue;
		}

		// ������� ���� ��������� ����� (����� ��������� ������ ������)
		static__buffer_clear(_cp);

		// ������� �� com- � usb-������
		if (!static__check_com(find_info.com, _cp))
			continue;
		if (!static__check_usb(find_info.usb, port))
			continue;

		cp = std::move(_cp);
		return port;
	}
	return 0;
}

/*static*/ bool device::static__check_com(
	_in const find_info::com_t &com, _in const com::port &cp
) {
	// ����� ��� ������ � com-�����
	com::at::result result;

	// ������� at-����������		
	static__at(cp, "AT", result.data);
	if (!com::at::check(result.data).find(result.match))
		return false;
	assert(1 == result.match.size());

	// ������� at-���������� ������������� 'HUAWEI'...
	static__at(cp, "AT+CGMI", result.data);
	if (!com::at::check(result.data).find(result.match))
		return false;
	assert(2 == result.match.size());
	if (com.vendor != string::to_lower(result.match.at(1)).string())
		return false;

	// ...� � ������ 'E173'
	static__at(cp, "AT+CGMM", result.data);
	if (!com::at::check(result.data).find(result.match))
		return false;
	assert(2 == result.match.size());
	if (com.model != string::to_lower(result.match.at(1)).string())
		return false;

	return true;
}

/*static*/ bool device::static__check_usb(
	_in const find_info::usb_t &usb, _in com::port::number cpn
) {
	assert(cpn < 0x100);
	const auto cp_name = L"COM" + std::to_wstring(cpn);

	HKEY hKey;
	auto Result = Winapi::RegOpenKeyExW(HKEY_LOCAL_MACHINE, LR"(SYSTEM\ControlSet001\Control\Class\{4D36E978-E325-11CE-BFC1-08002BE10318})", 0, KEY_ENUMERATE_SUB_KEYS, &hKey);
	if (ERROR_SUCCESS != Result) {
		trace(L"RegOpenKeyExW(KEY_ENUMERATE_SUB_KEYS): E(0x%X)", Result);
		Winapi::SetLastError(Result);
		return false;
	}
	std::unique_ptr<std::remove_pointer<decltype(hKey)>::type, decltype(&Winapi::RegCloseKey)> Key(hKey, &Winapi::RegCloseKey);
	
	char_t Name[] = L"XXXX";
	for (DWORD i = 0; ; ++i) {
		DWORD NameSize = _countof(Name);
		Result = Winapi::RegEnumKeyExW(hKey, i, Name, &NameSize, NULL, NULL, NULL, NULL);
		switch (Result) {
		case ERROR_SUCCESS:
			if (_countof(Name)-1 != NameSize)
				break;
			{
				HKEY hSubKey;
				Result = Winapi::RegOpenKeyExW(hKey, Name, 0, KEY_QUERY_VALUE, &hSubKey);
				if (ERROR_SUCCESS != Result) {
					trace(L"RegOpenKeyExW(KEY_QUERY_VALUE): E(0x%X)", Result);
					break;
				}
				std::unique_ptr<std::remove_pointer<decltype(hSubKey)>::type, decltype(&Winapi::RegCloseKey)> SubKey(hSubKey, &Winapi::RegCloseKey);

				byte_t Data[512];
				
				DWORD DataSize = _countof(Data);
				Result = Winapi::RegGetValueW(hSubKey, NULL, L"AssignedPortForQCDevice", RRF_RT_REG_SZ, NULL, Data, &DataSize);
				if (ERROR_SUCCESS != Result) {
					trace(L"RegGetValueW(\'AssignedPortForQCDevice\'): E(0x%X)", Result);
					break;
				}
				
				auto DataT = reinterpret_cast<cstr_t>(Data);
				if (DataT != cp_name)
					break;

				DataSize = _countof(Data);
				Result = Winapi::RegGetValueW(hSubKey, NULL, L"DriverDesc", RRF_RT_REG_SZ, NULL, Data, &DataSize);
				if (ERROR_SUCCESS != Result) {
					trace(L"RegGetValueW(\'DriverDesc\'): E(0x%X)", Result);
					break;
				}

				//auto DataT = reinterpret_cast<cstr_t>(Data);
				return DataT == usb.driver_desc;
			}
			break;
		case ERROR_NO_MORE_ITEMS:
			return false;
		default:			// some error or buffer(Name) is small
			trace(L"RegEnumKeyExW(): E(0x%X)", Result);
			break;
		}
	}

	assert(true);
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


/*static*/ cstr_at device::static__com__check_for_data(
	_in const com::port &cp, _out string_at &data
) {
	return cp.recieve(data);
}
/*static*/ string_at device::static__com__check_for_data(
	_in const com::port &cp
) {
	return cp.recieve();
}

cstr_at device::check_for_data(
	_out string_at &data
) const {
	return static__com__check_for_data(_cp, data);
}
string_at device::check_for_data(
) const {
	return static__com__check_for_data(_cp);
}


/*static*/ void device::static__buffer_clear(
	_in const com::port &cp
) {
	string_at data__not_used;
	static__com__check_for_data(cp, data__not_used);
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
	
	// �������� 2 ��������: �������� ������ - 3 match'� � ������ ������ (������� ������� ������) - 1 match
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
