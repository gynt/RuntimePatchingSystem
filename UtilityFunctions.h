#pragma once

#include "framework.h"
#include <iostream>
#include "lua.hpp"

extern lua_State* L;

#ifdef _DEBUG

bool canWrite(DWORD address, int length);

#endif
