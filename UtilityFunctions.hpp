#pragma once

#include "framework.h"
#include <iostream>
#include "lua.hpp"
#include <sstream>
#include <string>

extern lua_State* LC;

#ifdef _DEBUG

bool canWrite(DWORD address, int length);

#endif

typedef struct ByteStream {
	void* address;
	size_t len;
} ByteStream;

int convertTableToByteStream(lua_State* L, ByteStream * stream);