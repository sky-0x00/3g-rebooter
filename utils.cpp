#include "pch.h"

#include "utils.h"
#include <windows.h>
#include <cassert>
#include <regex>

#define Winapi

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
	_in number number, _in const config &config
) {
	const auto handle = Winapi::CreateFileW((LR"(\.\COM)" + std::to_wstring(number)).c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == handle) {
		trace(L"CreateFileW(): E(0x%X)", Winapi::GetLastError());
		return nullptr;
	}
	
	DCB dcb {sizeof(dcb)};
	auto IsOk = GetCommState(handle, &dcb);
	assert(IsOk);

	dcb.BaudRate = static_cast<decltype(dcb.BaudRate)>(config.baud_rate);
	dcb.ByteSize = static_cast<decltype(dcb.ByteSize)>(config.byte_size);
	dcb.Parity   = static_cast<decltype(dcb.Parity)>(config.parity);
	dcb.StopBits = static_cast<decltype(dcb.StopBits)>(config.stop_bits);

	IsOk = SetCommState(handle, &dcb);
	assert(IsOk);

	return handle;
}
/*static*/ set_lasterror(bool) com::port::static__close(
	_in handle handle
) {
	//if (!handle) {
	//	Winapi::SetLastError(ERROR_SUCCESS);
	//	return false;
	//}
	if (FALSE != Winapi::CloseHandle(handle))
		return true;
	trace(L"CloseHandle(): E(0x%X)", Winapi::GetLastError());
	return false;
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
	_handle = static__open(number, config);
	return nullptr != _handle;
}
set_lasterror(bool) com::port::close(
) {
	if (!static__close(_handle))
		return false;
	_handle = nullptr;
	return true;
}