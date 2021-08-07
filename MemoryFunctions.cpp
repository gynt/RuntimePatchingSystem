
#include "MemoryFunctions.h"

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
	int maxLength = 0;
	int length = 0;
	bool wide = false;
	if (lua_gettop(L) == 0) {
		return luaL_error(L, "too few arguments passed to readString");
	}
	address = lua_tointeger(L, 1);
	if (address == 0) {
		return luaL_error(L, "argument 1 must be a valid address");
	}

	if (lua_gettop(L) == 2) {
		maxLength = lua_tointeger(L, 2);
	}
	if (lua_gettop(L) == 3) {
		wide = lua_tointeger(L, 3) == 1;
	}

	if (wide || maxLength) {
		return luaL_error(L, "sorry, maxlength and wide are not supported yet.");
	}

	std::string result((char*)address);
	lua_pushstring(L, result.c_str());

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

	std::string value = lua_tostring(L, 2);

#ifdef _DEBUG
	if (!canWrite(address, value.size())) {
		return luaL_error(L, ("cannot write " + std::to_string(1) + " string to location: " + std::to_string(address)).c_str());
	}
#endif

	memcpy((void*)address, value.c_str(), value.size());
	
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

	int i = 0;

	lua_pushvalue(L, 2); // push the table again; so that it is at -1

	/* table is in the stack at index 't' */
	lua_pushnil(L);  /* first key */
	while (lua_next(L, -2) != 0) {
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		if (!lua_isinteger(L, -1)) {
			//lua_pushliteral(L, "The return value table must have integer values");
			//lua_error(L);
			return luaL_error(L, "The return value table must have integer values");
		}

		int value = lua_tointeger(L, -1);
		if (value < 0) {
			return luaL_error(L, "The values must all be positive");
		}

		*((BYTE*)(address + i)) = value;
		i += 1;

		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(L, 1);
	}
	//lua_next pops the key from the stack, if we reached the end, there is no key on the stack.

	lua_pop(L, 1); //removes 'table'

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