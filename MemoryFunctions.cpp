
#include "MemoryFunctions.h"

#include "CodeFunctions.h"

int luaReadByte(lua_State* L) {
	if (lua_gettop(L) != 1) {
		return luaL_error(L, "expected exactly 1 argument");
	}
	DWORD address = lua_tointeger(L, 1);
	if (address == 0) {
		return luaL_error(L, "argument 1 must be a valid address");
	}
	lua_pushinteger(L, *((BYTE*)address));
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

	lua_pushinteger(L, *((SHORT*)address));
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

	lua_pushinteger(L, *((int*)address));
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

	if (length > 0) {
		lua_pushlstring(L, (const char*)address, length);
	}
	else {
		// Finds the first \0 byte and terminates
		std::string result((const char*)address);
		lua_pushstring(L, result.c_str());
	}

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

	for (int i = 0; i < size; i++) {
		unsigned char value = *((BYTE*)(address + i));
		lua_pushinteger(L, (lua_Integer)i + 1);
		lua_pushinteger(L, value);
		lua_settable(L, -3);  /* 3rd element from the stack top */
	}

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
	std::string value = lua_tolstring(L, 2, &size);

#ifdef _DEBUG
	if (!canWrite(address, size)) {
		return luaL_error(L, ("cannot write " + std::to_string(1) + " string to location: " + std::to_string(address)).c_str());
	}
#endif

	memcpy((void*)address, value.data(), size);
	
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

#ifdef _DEBUG
	if (!canWrite(address, 1)) {
		return luaL_error(L, ("cannot write " + std::to_string(1) + " bytes to location: " + std::to_string(address)).c_str());
	}
#endif

	* ((BYTE*)address) = value;
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

#ifdef _DEBUG
	if (!canWrite(address, 2)) {
		return luaL_error(L, ("cannot write " + std::to_string(2) + " bytes to location: " + std::to_string(address)).c_str());
	}
#endif

	* ((SHORT*)address) = value;
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

#ifdef _DEBUG
	if (!canWrite(address, 4)) {
		return luaL_error(L, ("cannot write " + std::to_string(4) + " bytes to location: " + std::to_string(address)).c_str());
	}
#endif

	* ((int*)address) = value;
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

#ifdef _DEBUG
	int length = lua_rawlen(L, 2);
	if (!canWrite(address, length)) {
		return luaL_error(L, ("cannot write " + std::to_string(length) + " bytes to location: " + std::to_string(address)).c_str());
	}
#endif

	// Makes use the of the table at -1 (2)
	std::stringstream bytes;
	int returnCode = convertTableToByteStream(L, &bytes);

	if (returnCode == -1) {
		return luaL_error(L, "The return value table must have integer values");
	}
	else if (returnCode == -2) {
		return luaL_error(L, "The values must all be positive");
	}

	bytes.seekg(0, bytes.end);
	int size = bytes.tellg();
	bytes.seekg(0, bytes.beg);

	// str() is null-terminated, but size is the size without the final null byte, which makes this correct
	memcpy((void*)address, bytes.str().data(), size);

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

#ifdef _DEBUG
	if (!canWrite(dst, size)) {
		return luaL_error(L, ("cannot write " + std::to_string(size) + " bytes to location: " + std::to_string(dst)).c_str());
	}
#endif

	memcpy((void*)dst, (void*)src, size);

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

	DWORD val = lua_tointeger(L, 2);
	if (val == 0) {
		return luaL_error(L, "argument 2 must be a valid integer");
	}

	int size = lua_tointeger(L, 3);
	if (size == 0) {
		return luaL_error(L, "argument 3 must be a valid size higher than 0");
	}

#ifdef _DEBUG
	if (!canWrite(dst, size)) {
		return luaL_error(L, ("cannot write " + std::to_string(size) + " bytes to location: " + std::to_string(dst)).c_str());
	}
#endif

	memset((void*)dst, val, size);

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
	if (lua_gettop(L) == 2 && lua_toboolean(L, 2)) {
		memory = calloc(size, sizeof(BYTE));
	}
	else {
		memory = malloc(size);
	}

	lua_pushinteger(L, (DWORD_PTR)memory);

	return 1;
}

int luaDeallocate(lua_State* L) {
	if (lua_gettop(L) != 1) {
		return luaL_error(L, "Expected one argument");
	}

	int addr = luaL_checkinteger(L, 1);
	if (addr == 0) {
		return luaL_error(L, "Address is 0");
	}

	void* memory = (void* )((DWORD_PTR) addr);
	free(memory);

	return 0;
}

