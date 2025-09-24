#include "UtilityFunctions.hpp"

lua_State* LC = 0;

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


int convertTableToByteStream(lua_State* L, ByteStream * stream) {
	std::stringstream s;

	for (int i = 1; ; i++) {
		lua_geti(L, -1, i);

		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
			break;
		}

		if (!lua_isinteger(L, -1)) {
			lua_pop(L, 1);
			return -1;
		}

		unsigned int value = lua_tointeger(L, -1);

		if (value <= 0xff && value >= 0x00) {
			s.write(reinterpret_cast<const char*>(&value), 1);
		}
		else {
			s.write(reinterpret_cast<const char*>(&value), 4);
		}

		/* removes 'value' */
		lua_pop(L, 1);
	}

	s.seekg(0, s.end);
	int size = s.tellg ();
	s.seekg(0, s.beg);

	stream->address = calloc(size, 1);
	if (stream->address == NULL) {
		return -1;
	}
	stream->len = size;
	memcpy(stream->address, s.str().data(), size);

	return 0;
}