#include "UtilityFunctions.h"

lua_State* L = 0;

#ifdef _DEBUG

bool canWrite(DWORD address, int length) {
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	MEMORY_BASIC_INFORMATION mi;
	SIZE_T vq = VirtualQuery((void*)address, &mi, sizeof(mi));
	if (vq == ERROR_INVALID_PARAMETER || vq == 0) {
		std::cout << "ERROR CODE: " << GetLastError() << std::endl;
		return false;
	}

	return mi.Protect & (PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY | PAGE_WRITECOMBINE | PAGE_WRITECOPY | PAGE_READWRITE);
}

#endif