#include "bitness.h"
#include <windows.h>
//#include <cassert>
#include "pch.h"

#ifdef interface
	#undef interface
#endif

//--------------------------------------------------------------------------------------------------------------------------
/*virtual*/ bitness::value bitness::interface::get(
) /*final*/ {
	if (!_value.get())
		_value.reset(new value(static__get()));
	return *_value;
}

//--------------------------------------------------------------------------------------------------------------------------
/*static*/ constexpr bitness::value bitness::process::static__get(
) noexcept {
#if _WIN64
	return x64;
#else 
	return x32;
#endif
}

/*static*/ bool bitness::process::static__is_wow64(
) {
	BOOL IsWow64;
	if (FALSE == Winapi::IsWow64Process(Winapi::GetCurrentProcess(), &IsWow64)) {
		const auto LastError = Winapi::GetLastError();
		trace(L"IsWow64Process(): E(0x%X)", LastError);
		throw LastError;
	}

	return FALSE != IsWow64;
}
bool bitness::process::is_wow64(
) {
	if (!_is_wow64.get())
		_is_wow64.reset(new bool(static__is_wow64()));
	return *_is_wow64;
}

/*static*/ bitness::value bitness::system::static__get(
) {
	// x64-������� ����� ����������� ������ � ����� x64-�������
	if (x64 == process::static__get())
		return x64;

	return process::static__is_wow64() ? x64 : x32;
}

//--------------------------------------------------------------------------------------------------------------------------
