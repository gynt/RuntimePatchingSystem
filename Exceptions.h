#pragma once

#include "framework.h"
#include <string>
#include <sstream>

#define RPS_HANDLE_SEH errorFilterAndReporter(GetExceptionCode(), GetExceptionInformation())
#define RPS_LUA_SEH luaL_error(L, "%s", errorReport)
#define RPS_LUA_SEH_X(x) luaL_error(L, "%s  while executing 0x%X", errorReport, x)
#define RPS_LUA_SEH_ADDRESS luaL_error(L, "%s  while executing 0x%X", errorReport, address)

extern const char errorReport[1000];
int errorFilterAndReporter(unsigned int code, struct _EXCEPTION_POINTERS* ep);