
#include "MemoryFunctions.h"

#include "CodeFunctions.h"
#include "Exceptions.h"
#include "UtilityFunctions.hpp"

int luaReadByte(lua_State* L) {
	if (lua_gettop(L) != 1) {
		return luaL_error(L, "expected exactly 1 argument");
	}
	DWORD address = lua_tointeger(L, 1);
	if (address == 0) {
		return luaL_error(L, "argument 1 must be a valid address");
	}
	
#ifdef EH_GUARDRAILS
	__try {
#endif
		lua_pushinteger(L, *((BYTE*)address));
#ifdef EH_GUARDRAILS
	}
	__except (RPS_HANDLE_SEH) {
		return RPS_LUA_SEH_ADDRESS;
	}
#endif
	return 1;
}

int luaReadSmallInteger(lua_State* L) {
	if (lua_gettop(L) != 1) {
		return luaL_error(L, "expected exactly 1 argument");
	}
	DWORD address = lua_tointeger(L, 1);
	if (address == 0) {
		return luaL_error(L, "argument 1 must be a valid address");
	}

#ifdef EH_GUARDRAILS
	__try {
#endif
		lua_pushinteger(L, *((SHORT*)address));
#ifdef EH_GUARDRAILS
	}
	__except (RPS_HANDLE_SEH) {
		return RPS_LUA_SEH_ADDRESS;
	}
#endif
	return 1;
}

int luaReadInteger(lua_State* L) {
	if (lua_gettop(L) != 1) {
		return luaL_error(L, "expected exactly 1 argument");
	}
	DWORD address = lua_tointeger(L, 1);
	if (address == 0) {
		return luaL_error(L, "argument 1 must be a valid address");
	}

#ifdef EH_GUARDRAILS
	__try {
#endif
		lua_pushinteger(L, *((int*)address));
#ifdef EH_GUARDRAILS
	}
	__except (RPS_HANDLE_SEH) {
		return RPS_LUA_SEH_ADDRESS;
	}
#endif
	return 1;
}

int luaReadString(lua_State* L) {
	DWORD address = 0;
	size_t length = 0;
	bool wide = false;
	if (lua_gettop(L) == 0) {
		return luaL_error(L, "too few arguments passed to readString");
	}
	address = lua_tointeger(L, 1);
	if (address == 0) {
		return luaL_error(L, "argument 1 must be a valid address");
	}

	if (lua_gettop(L) == 2) {
		if (!lua_isnil(L, 2)) {
			length = lua_tointeger(L, 2);
		}
	}
	if (lua_gettop(L) == 3) {
		wide = lua_tointeger(L, 3) == 1;
	}

	if (wide) {
		return luaL_error(L, "sorry, wide string is not supported yet.");
	}

#ifdef EH_GUARDRAILS
	__try {
#endif
		if (length > 0) {
			lua_pushlstring(L, (const char*)address, length);

		}
		else {
			// Finds the first \0 byte and terminates
			lua_pushlstring(L, (const char *)address, strlen((const char*) address));
		}
#ifdef EH_GUARDRAILS
	}
	__except (RPS_HANDLE_SEH) {
		return RPS_LUA_SEH_ADDRESS;
	}
#endif


	return 1;
}

int luaReadBytes(lua_State* L) {
	if (lua_gettop(L) != 2) {
		return luaL_error(L, "expected exactly 2 arguments");
	}

	DWORD address = lua_tointeger(L, 1);
	if (address == 0) {
		return luaL_error(L, "argument 1 must be a valid address");
	}

	int size = lua_tointeger(L, 2);

	lua_createtable(L, size, 0);

#ifdef EH_GUARDRAILS
	__try {
#endif
		for (int i = 0; i < size; i++) {
			unsigned char value = *((BYTE*)(address + i));


			lua_pushinteger(L, (lua_Integer)i + 1);
			lua_pushinteger(L, value);
			lua_settable(L, -3);  /* 3rd element from the stack top */
		}
#ifdef EH_GUARDRAILS
	}
	__except (RPS_HANDLE_SEH) {
		// address is technically not entirely correct, but will probably point
		// in the right direction to fix the underlying cause
		return RPS_LUA_SEH_ADDRESS;
	}
#endif


	// we pass the table back;

	return 1;
}

int luaWriteString(lua_State* L) {
	if (lua_gettop(L) != 2) {
		return luaL_error(L, "expected exactly 2 arguments");
	}
	DWORD address = lua_tointeger(L, 1);
	if (address == 0) {
		return luaL_error(L, "argument 1 must be a valid address");
	}

	size_t size = 0;
	const char* value = lua_tolstring(L, 2, &size);

#ifdef EH_GUARDRAILS
	__try {
#endif
		memcpy((void*)address, value, size);
#ifdef EH_GUARDRAILS
	}
	__except (RPS_HANDLE_SEH) {
		return RPS_LUA_SEH_ADDRESS;
	}
#endif

	

	return 0;
}

int luaWriteByte(lua_State* L) {
	if (lua_gettop(L) != 2) {
		return luaL_error(L, "expected exactly 2 arguments");
	}
	DWORD address = lua_tointeger(L, 1);
	if (address == 0) {
		return luaL_error(L, "argument 1 must be a valid address");
	}

	BYTE value = lua_tointeger(L, 2);

#ifdef EH_GUARDRAILS
	__try {
#endif
		* ((BYTE*)address) = value;
#ifdef EH_GUARDRAILS
	}
	__except (RPS_HANDLE_SEH) {
		return RPS_LUA_SEH_ADDRESS;
	}
#endif

	return 0;
}

int luaWriteSmallInteger(lua_State* L) {
	if (lua_gettop(L) != 2) {
		return luaL_error(L, "expected exactly 2 arguments");
	}
	DWORD address = lua_tointeger(L, 1);
	if (address == 0) {
		return luaL_error(L, "argument 1 must be a valid address");
	}

	SHORT value = lua_tointeger(L, 2);

#ifdef EH_GUARDRAILS
	__try {
#endif
		* ((SHORT*)address) = value;
#ifdef EH_GUARDRAILS
	}
	__except (RPS_HANDLE_SEH) {
		return RPS_LUA_SEH_ADDRESS;
	}
#endif

	
	return 0;
}

int luaWriteInteger(lua_State* L) {
	if (lua_gettop(L) != 2) {
		return luaL_error(L, "expected exactly 2 arguments");
	}
	DWORD address = lua_tointeger(L, 1);
	if (address == 0) {
		return luaL_error(L, "argument 1 must be a valid address");
	}

	int value = lua_tointeger(L, 2);

#ifdef EH_GUARDRAILS
	__try {
#endif
		* ((int*)address) = value;
#ifdef EH_GUARDRAILS
	}
	__except (RPS_HANDLE_SEH) {
		return RPS_LUA_SEH_ADDRESS;
	}
#endif

	
	return 0;
}



int luaWriteBytes(lua_State* L) {
	if (lua_gettop(L) != 2) {
		return luaL_error(L, "expected exactly 2 arguments");
	}
	DWORD address = lua_tointeger(L, 1);
	if (address == 0) {
		return luaL_error(L, "argument 1 must be a valid address");
	}

	if (!lua_istable(L, 2)) {
		return luaL_error(L, "the second argument should be a table");
	}


	// Makes use the of the table at -1 (2)
	ByteStream stream;
	int returnCode = convertTableToByteStream(L, &stream);

	if (returnCode == -1) {
		return luaL_error(L, "The return value table must have integer values");
	}
	else if (returnCode == -2) {
		return luaL_error(L, "The values must all be positive");
	}

#ifdef EH_GUARDRAILS
	__try {
#endif
		// str() is null-terminated, but size is the size without the final null byte, which makes this correct
		memcpy((void*)address, stream.address, stream.len);
#ifdef EH_GUARDRAILS
	}
	__except (RPS_HANDLE_SEH) {
		return RPS_LUA_SEH_ADDRESS;
	}
#endif

	free(stream.address);

	return 0;
}

int luaMemCpy(lua_State* L) {

	if (lua_gettop(L) != 3) {
		return luaL_error(L, "expected exactly 3 arguments");
	}

	DWORD dst = lua_tointeger(L, 1);
	if (dst == 0) {
		return luaL_error(L, "argument 1 must be a valid address");
	}
	
	DWORD src = lua_tointeger(L, 2);
	if (src == 0) {
		return luaL_error(L, "argument 2 must be a valid address");
	}

	int size = lua_tointeger(L, 3);
	if (size == 0) {
		return luaL_error(L, "argument 3 must be a valid size higher than 0");
	}

#ifdef EH_GUARDRAILS
	__try {
#endif
		memcpy((void*)dst, (void*)src, size);
#ifdef EH_GUARDRAILS
	}
	__except (RPS_HANDLE_SEH) {
		return RPS_LUA_SEH_X(dst);
	}
#endif
	

	return 0;
}


int luaMemSet(lua_State* L) {

	if (lua_gettop(L) != 3) {
		return luaL_error(L, "expected exactly 3 arguments");
	}

	DWORD dst = lua_tointeger(L, 1);
	if (dst == 0) {
		return luaL_error(L, "argument 1 must be a valid address");
	}

	if (lua_type(L, 2) != LUA_TNUMBER) {
		return luaL_error(L, "argument 2 must be a valid integer");
	}
	DWORD val = lua_tointeger(L, 2);


	int size = lua_tointeger(L, 3);
	if (size == 0) {
		return luaL_error(L, "argument 3 must be a valid size higher than 0");
	}


#ifdef EH_GUARDRAILS
	__try {
#endif
		memset((void*)dst, val, size);
#ifdef EH_GUARDRAILS
	}
	__except (RPS_HANDLE_SEH) {
		return RPS_LUA_SEH_X(dst);
	}
#endif
	

	return 0;
}


std::set<std::string> stringSet;

int registerString(lua_State* L) {
	if (lua_gettop(L) != 1) {
		return luaL_error(L, "Wrong number of arguments passed");
	}

	std::string target = lua_tostring(L, 1);
	std::pair<std::set<std::string>::iterator, bool> p = stringSet.insert(target);

	std::set<std::string>::iterator it = p.first;
	lua_pushinteger(L, (DWORD)p.first->c_str());

	return 1;
}



int luaAllocate(lua_State* L) {
	if (lua_gettop(L) != 1 && lua_gettop(L) != 2) {
		return luaL_error(L, "Expected one or two arguments");
	}

	void* memory;

	int size = lua_tonumber(L, 1);
#ifdef EH_GUARDRAILS
	__try {
#endif
		if (lua_gettop(L) == 2 && lua_toboolean(L, 2)) {
			memory = calloc(size, sizeof(BYTE));
		}
		else {
			memory = malloc(size);
		}
#ifdef EH_GUARDRAILS
	}
	__except (RPS_HANDLE_SEH) {
		return RPS_LUA_SEH;
	}
#endif


	lua_pushinteger(L, (DWORD_PTR)memory);

	return 1;
}

int luaDeallocate(lua_State* L) {
	if (lua_gettop(L) != 1) {
		return luaL_error(L, "Expected one argument");
	}

	int address = luaL_checkinteger(L, 1);
	if (address == 0) {
		return luaL_error(L, "Address is 0");
	}

	void* memory = (void* )((DWORD_PTR) address);

#ifdef EH_GUARDRAILS
	__try {
#endif
		free(memory);
#ifdef EH_GUARDRAILS
	}
	__except (RPS_HANDLE_SEH) {
		return RPS_LUA_SEH_ADDRESS;
	}
#endif
	

	return 0;
}

