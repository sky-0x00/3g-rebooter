#include "pch.h"

#include "utils.h"
#include <windows.h>
#include <cassert>
#include <regex>
#include <cctype>
#include <algorithm>

//------------------------------------------------------------------------------------------------------------------------------------------------------
cstr_at string::psize_a(
	_in cstr_at src
) {
	while ('\0' != *src) ++src;
	return src;
}
unsigned string::size_a(
	_in cstr_at src
) {
	return psize_a(src) - src;
}

str_at string::pcopy_a(
	_out str_at dst, _in unsigned dst_size, _in cstr_at src
) {
	for (; dst_size--; ++dst, ++src) {
		*dst = *src;
		if ('\0' == *dst)
			return dst;
	}
	return dst;
}
unsigned string::copy_a(
	_out str_at dst, _in unsigned dst_size, _in cstr_at src
) {
	return pcopy_a(dst, dst_size, src) - dst;
}

static void to_case(
	_in int(__cdecl *f)(int), _in string_at::const_iterator cbegin, _in string_at::const_iterator cend, _out string_at::iterator begin
) {
	std::transform(cbegin, cend, begin, [f](_in uchar_at ch) -> char_at {
		return f(ch);
	});
}

const string_at& string::to_upper(
	_in _out string_at &string
) {
	::to_case(std::toupper, string.cbegin(), string.cend(), string.begin());
	return string;
}
const string_at& string::to_lower(
	_in _out string_at &string
) {
	::to_case(std::tolower, string.cbegin(), string.cend(), string.begin());
	return string;
}

const string::view_at<string_at::const_iterator>& string::to_upper(
	_in _out string::view_at<string_at::const_iterator> &string_view
) {
	// нужно как-то создать обычный итератор из константного итератора; это нехорошо, но если очень нужно, то...
	string_at::iterator it(const_cast<str_at>(&*string_view.first), string_view.first._Getcont());

	::to_case(std::toupper, string_view.first, string_view.second, it);
	return string_view;
}
const string::view_at<string_at::const_iterator>& string::to_lower(
	_in _out string::view_at<string_at::const_iterator> &string_view
) {
	// нужно как-то создать обычный итератор из константного итератора; это нехорошо, но если очень нужно, то...
	string_at::iterator it(const_cast<str_at>(&*string_view.first), string_view.first._Getcont());

	::to_case(std::tolower, string_view.first, string_view.second, it);
	return string_view;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------
com::port::buffer::buffer(
) :
	size(0)
{}
com::port::buffer::buffer(
	_in _out buffer &&buffer
) :
	size(buffer.size)
{
	data.swap(buffer.data);
}

com::port::buffer& com::port::buffer::operator =(
	_in _out buffer &&buffer
) {
	size = buffer.size;
	data.swap(buffer.data);
	return *this;
}

bool com::port::buffer::is_empty(
) const {
	return (nullptr == data.get()) && (0 == size);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------
com::port::config::config(
	_in const dcb_t &dcb, _in timeouts_t timeouts, _in unsigned buffer_size /*= 4096*/
) :
	dcb(dcb), timeouts(timeouts), buffer_size(buffer_size)
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
	_in number number, _in const config::dcb_t &config_dcb, _in const config::timeouts_t &timeouts
) {
	const auto handle = Winapi::CreateFileW((LR"(\.\COM)" + std::to_wstring(number)).c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == handle) {
		trace(L"CreateFileW(): E(0x%X)", Winapi::GetLastError());
		return nullptr;
	}
	
	{
		DCB dcb{ sizeof(dcb) };
		auto IsOk = FALSE != Winapi::GetCommState(handle, &dcb);
		assert(IsOk);

		dcb.BaudRate = static_cast<decltype(dcb.BaudRate)>(config_dcb.baud_rate);
		dcb.ByteSize = static_cast<decltype(dcb.ByteSize)>(config_dcb.byte_size);
		dcb.Parity   = static_cast<decltype(dcb.Parity)>(config_dcb.parity);
		dcb.StopBits = static_cast<decltype(dcb.StopBits)>(config_dcb.stop_bits);

		IsOk = FALSE != Winapi::SetCommState(handle, &dcb);
		assert(IsOk);
	}

	{
		COMMTIMEOUTS ct;
		auto IsOk = FALSE != Winapi::GetCommTimeouts(handle, &ct);
		assert(IsOk);

		ct.ReadIntervalTimeout         = static_cast<decltype(ct.ReadIntervalTimeout)>(timeouts.read.interval);
		ct.ReadTotalTimeoutMultiplier  = static_cast<decltype(ct.ReadTotalTimeoutMultiplier)>(timeouts.read.total.multiplier);
		ct.ReadTotalTimeoutConstant    = static_cast<decltype(ct.ReadTotalTimeoutConstant)>(timeouts.read.total.constant);
		ct.WriteTotalTimeoutMultiplier = static_cast<decltype(ct.WriteTotalTimeoutMultiplier)>(timeouts.write.multiplier);
		ct.WriteTotalTimeoutConstant   = static_cast<decltype(ct.WriteTotalTimeoutConstant)>(timeouts.write.constant);

		IsOk = FALSE != Winapi::SetCommTimeouts(handle, &ct);
		assert(IsOk);
	}

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
com::port::port(
	_in _out port &&port
) :
	_handle( std::exchange(port._handle, nullptr) ), _buffer(std::move(port._buffer))
{}

com::port::~port(
) {
	if (_handle)
		Winapi::CloseHandle(_handle);
}

com::port& com::port::operator =(
	_in _out port &&port
) {
	_handle = std::exchange(port._handle, nullptr);
	_buffer = std::move(port._buffer);
	return *this;
}

///*static*/ bool com::port::static__compare(
//	_in const port &lhs, _in const port &rhs
//) {
//	return lhs._handle == rhs._handle;
//}
//bool com::port::compare(
//	_in const port &port
//) const {
//	return static__compare(*this, port);
//}

//bool com::port::operator ==(
//	_in const port &port
//) const {
//	return compare(port);
//}
//bool com::port::operator !=(
//	_in const port &port
//) const {
//	return !compare(port);
//}

set_lasterror(bool) com::port::open(
	_in number number, _in const config &config
) {
	_handle = static__open(number, config.dcb, config.timeouts);
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

/*static*/ set_lasterror(unsigned) com::port::static__send(
	_in handle handle, _in const buffer &buffer, _in unsigned count
) {
	assert(handle && (0 < count));

	// передаем все символы до символа '\0', заменив последий символом '\n'
	const auto data = buffer.data.get();
	std::pair<DWORD, DWORD> size {count, 0};
	
	if ('\r' != data[size.first])
		data[size.first++] = '\r';
	const auto IsOk = FALSE != Winapi::WriteFile(handle, data, size.first, &size.second, nullptr);
	//if (count != size.first)... data[size.first--] = '\0';
	assert(IsOk);
	//if (!IsOk) {
	//	trace(L"WriteFile(), E(0x%X)", Winapi::GetLastError());
	//	...;
	//}

	assert(size.first == size.second);
	return /*size.first or*/ size.second;
}

set_lasterror(unsigned) com::port::send(
	_in cstr_at str
) const {
	assert(!_buffer.is_empty());
	const auto count = string::copy_a(_buffer.data.get(), _buffer.size, str);
	if (0 == count)
		return 0;
	return static__send(_handle, _buffer, count);
}
set_lasterror(unsigned) com::port::send(
	_in const string_at &string
) const {
	return send(string.c_str());
}


/*static*/ set_lasterror(unsigned) com::port::static__recieve(
	_in handle handle, _out buffer &buffer
) {
	std::pair<const str_at, str_at> data { buffer.data.get(), buffer.data.get() };
	auto result = [&data]() -> unsigned {
		return data.second - data.first;
	};

	assert(handle);
	for (DWORD char_readed = 0; ;) {

		// читаем все символы до спец-символа (блокирующая операция)
		const bool IsOk = Winapi::ReadFile(handle, data.second, 1, &char_readed, nullptr);
		assert(IsOk);
		//if (!IsOk) {
		//	trace(L"ReadFile(), E(0x%X)", Winapi::GetLastError());
		//	return result;
		//}

		if (1 != char_readed)
			return result();

		if ('\r' == *data.second)
			continue;
		
		++data.second;		
		assert(result() < buffer.size);
	}	
	assert(true);		// never get here
}

set_lasterror(cstr_at) com::port::recieve(
	_out string_at &string
) const {
	assert(!_buffer.is_empty());
	const auto count = static__recieve(_handle, _buffer);
	string.assign(_buffer.data.get(), count);
	return string.c_str();
}
set_lasterror(string_at) com::port::recieve(
) const {
	string_at result;
	recieve(result);
	return result;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------
//com::at::result::result(
//	_in _out result &&result
//) {
//	data.swap(result.data);
//	match.swap(result.match);
//}
//com::at::result& com::at::result::operator =(
//	_in _out result &&result
//) {
//	data.swap(result.data);
//	match.swap(result.match);
//	return *this;
//}

void com::at::result::clear(
) {
	data.clear();
	match.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------
com::at::check::check(
	_in const string_at &string
) :
	_string(string)
{}

bool com::at::check::process(
	_in cstr_at regex_pattern, _out match &match
) const noexcept {
	match.clear();

	std::smatch sm;
	if (!std::regex_search(_string, sm, std::regex(regex_pattern)))
		return false;

	match.reserve(sm.size() - 1);
	for (auto it = sm.cbegin(); ++it != sm.cend(); )
		if (it->matched)
			match.emplace_back(it->first, it->second);

	return true;
}
//com::at::match com::at::check::process(
//	_in cstr_at regex_pattern
//) const {
//	match match;
//	const auto is_matched = process(regex_pattern, match);
//	if (!is_matched)
//		assert(match.empty());			// match может быть пустым в случае успеха
//	return match;
//}

bool com::at::check::find(
	_out match &match
) const {
	const auto is_matched = process(R"((.*)\n(?:(.+)\n\n)?OK\n)", match);
	assert(is_matched ? (stdex::is_any<unsigned>(match.size(), {1, 2})) : match.empty());
	return is_matched;
}

bool com::at::check::new_sms(
	_out match &match
) const {
	const auto is_matched = process(R"*(\n\+CMTI: "([A-Z]{2})",(\d+)\n)*", match);
	assert(is_matched ? (2 == match.size()) : match.empty());
	return is_matched;
}

bool com::at::check::sms_read(
	_out match &match
) const {
	const auto is_matched = process(R"((?:\n\+CMGR: ([0-3]{1}),,(\d+)\n([\dA-F]*)\n\nOK\n)|(?:\n\+CMS ERROR: (\d+)\n))", match);
	assert(is_matched ? (stdex::is_any<unsigned>(match.size(), {1, 3})) : match.empty());
	return is_matched;
}

bool com::at::check::sms_format(
	_out match &match
) const {
	const auto is_matched = process(R"(\n\+CMGF: ([01])\n\nOK\n)", match);
	assert(is_matched ? (1 == match.size()) : match.empty());
	return is_matched;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------
/*static*/ bool console::cursor_position::safe__get(
	_out value &value
) noexcept {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (FALSE == Winapi::GetConsoleScreenBufferInfo(Winapi::GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
		return false;
	value = { csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y };
	return true;
}
/*static*/ console::cursor_position::value console::cursor_position::get(
) {
	value value;
	if (safe__get(value))
		return value;
	throw Winapi::GetLastError();
}

/*static*/ bool console::cursor_position::safe__set(
	_in const value &value
) noexcept {
	const COORD coord {value.x, value.y};
	return FALSE != Winapi::SetConsoleCursorPosition(Winapi::GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
/*static*/ void console::cursor_position::set(
	_in const value &value
) {
	if (safe__set(value))
		return;
	throw Winapi::GetLastError();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------
