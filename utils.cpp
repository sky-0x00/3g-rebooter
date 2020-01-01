#include "pch.h"

#include "utils.h"
#include <windows.h>
#include <cassert>
#include <regex>

#define Winapi

com::port::config::config(
	_in const dcb_t &dcb, _in unsigned buffer_size /*= 4096*/
) :
	dcb(dcb), buffer_size(buffer_size)
{}

/*static*/ std::vector<com::port::number> com::port::static__enum(
) {
	// открытые порты перечислены в ключе реестра HKLM\HARDWARE\DEVICEMAP\SERIALCOMM в виде списка записей со значениями "COM1", "COM12", ...
	std::vector<com::port::number> result;

	HKEY hKey = nullptr;
	if (ERROR_SUCCESS != Winapi::RegOpenKeyExW(HKEY_LOCAL_MACHINE, LR"(HARDWARE\DEVICEMAP\SERIALCOMM)", 0, KEY_QUERY_VALUE, &hKey)) {
		trace(L"RegOpenKeyExW()");
		return result;
	}

	struct Info {
		DWORD ValueCount, MaxNameLength, MaxValueLength;
	} Info {};
	if (ERROR_SUCCESS != Winapi::RegQueryInfoKeyW(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &Info.ValueCount, &Info.MaxNameLength, &Info.MaxValueLength, NULL, NULL)) {
		RegCloseKey(hKey);
		trace(L"RegQueryInfoKeyW()");
		return result;
	}
	result.reserve(Info.ValueCount);
	
	struct Buffer {
		std::vector<char_t> Name, Value;
	} Buffer {
		std::vector<char_t>(1 + Info.MaxNameLength), std::vector<char_t>(Info.MaxValueLength >> 1)
	};
	for (decltype(Info.ValueCount) &i = Info.ValueCount; i--; ) {
		struct BufferLength {
			DWORD Name, Value;
		} BufferLength {
			static_cast<DWORD>(Buffer.Name.size()), static_cast<DWORD>(Buffer.Value.size() << 1)
		};
		DWORD Type = REG_NONE;
		if (ERROR_SUCCESS != Winapi::RegEnumValueW(hKey, i, Buffer.Name.data(), &BufferLength.Name, NULL, &Type, reinterpret_cast<LPBYTE>(Buffer.Value.data()), &BufferLength.Value)) {
			trace(L"RegEnumValueW()");
			continue;
		}
		if (REG_SZ != Type) {
			trace(L"Not REG_SZ");
			continue;
		}
		if (BufferLength.Value < 10) {
			trace(L"Buffer.Value.size() < 4 w-chars");
			continue;
		}
		
		std::wcmatch match;
		if (!std::regex_match(Buffer.Value.data(), match, std::wregex{ L"COM(\\d{1,3})" })) {
			trace(L"Not match COM<M>");
			continue;
		}

		result.push_back(_wtoi(match[1].first));
	}

	Winapi::RegCloseKey(hKey);
	return result;
}

/*static*/ set_lasterror(com::port::handle) com::port::static__open(
	_in number number, _in const config::dcb_t &config_dcb
) {
	const auto handle = Winapi::CreateFileW((LR"(\.\COM)" + std::to_wstring(number)).c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == handle) {
		trace(L"CreateFileW(): E(0x%X)", Winapi::GetLastError());
		return nullptr;
	}
	
	DCB dcb {sizeof(dcb)};
	auto IsOk = GetCommState(handle, &dcb);
	assert(IsOk);

	dcb.BaudRate = static_cast<decltype(dcb.BaudRate)>(config_dcb.baud_rate);
	dcb.ByteSize = static_cast<decltype(dcb.ByteSize)>(config_dcb.byte_size);
	dcb.Parity   = static_cast<decltype(dcb.Parity)>  (config_dcb.parity);
	dcb.StopBits = static_cast<decltype(dcb.StopBits)>(config_dcb.stop_bits);

	IsOk = SetCommState(handle, &dcb);
	assert(IsOk);

	return handle;
}
/*static*/ set_lasterror(com::port::handle) com::port::static__close(
	_in handle handle
) {
	//if (!handle) {
	//	Winapi::SetLastError(ERROR_SUCCESS);
	//	return false;
	//}
	if (FALSE != Winapi::CloseHandle(handle))
		return nullptr;
	trace(L"CloseHandle(): E(0x%X)", Winapi::GetLastError());
	return handle;
}

com::port::port(
) :
	_handle(nullptr)
{}
com::port::~port(
) {
	if (_handle)
		Winapi::CloseHandle(_handle);
}

set_lasterror(bool) com::port::open(
	_in number number, _in const config &config
) {
	_handle = static__open(number, config.dcb);
	if (!_handle)
		return false;

	_buffer.size = config.buffer_size;
	_buffer.data.reset(new buffer::char_t[_buffer.size]);
	assert(_buffer.data.get());
	
	return true;
}
set_lasterror(bool) com::port::close(
) {
	_handle = static__close(_handle);
	if (_handle)
		return false;

	_buffer.size = 0;
	_buffer.data.reset();
	assert(!_buffer.data.get());

	return true;
}

/*static*/ set_lasterror(com::port::buffer::size_t) com::port::static_send(
	_in handle handle, _in const buffer &buffer
) {
	const auto data = buffer.data.get();
	assert(handle && data);

	// передаем все символы до символа '\0', заменив последий символом '\n'
	std::pair<const DWORD, DWORD> size {strnlen_s(data, buffer.size), 0};
	data[size.first] = '\n';

	const auto IsOk = FALSE != Winapi::WriteFile(handle, data, size.first, &size.second, nullptr);
	data[size.first] = '\0';

	if (!IsOk)
		return 0;

	assert(size.first == size.second);
	return size.second /*or size.first*/;
}
/*static*/ set_lasterror(com::port::buffer::size_t) com::port::static_recieve(
	_in handle handle, _out buffer &buffer
) {
	buffer::size_t result = 0;

	auto data = buffer.data.get();
	assert(handle && data);

	// читаем все символы до символа '\n' (блокирующая операция)
	for (DWORD dummy = 0; result < buffer.size; ) {
		const bool IsOk = Winapi::ReadFile(handle, data, 1, &dummy, nullptr);
		assert(IsOk && (1 == dummy));
		switch (*data) {
		case '\n':
			*data = '\0';
			return result;
		case '\r':
			break;
		default:
			++data;
			++result;
		}
	}	
	return result;
}