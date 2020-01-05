#include "pch.h"
#include "system.h"
#include <windows.h>

/*static*/ set_lasterror(bool) windows::reboot::static__do(
	_in unsigned timeout /*= 0*/
) {
#ifdef _DEBUG
	// ok
	//return true;

	// fail, not have privilage
	Winapi::SetLastError(ERROR_ACCESS_DENIED);
	return false;
#else
	return FALSE != Winapi::InitiateSystemShutdownW(NULL, NULL, timeout, TRUE, TRUE);
#endif
}

/*static*/ bool windows::reboot::static__check_privs(
) {
	return true;
}