#pragma once

#include "framework.h"
#include <string>
#include <sstream>

#define RPS_HANDLE_SEH errorFilterAndReporter(GetExceptionCode(), GetExceptionInformation())
#define RPS_LUA_SEH luaL_error(L, "%s", errorReport);
#define RPS_LUA_SEH_X(x) (sprintf_s(intHex, "%X", x), luaL_error(L, "%s address: 0x%s", errorReport, intHex));
#define RPS_LUA_SEH_ADDRESS (sprintf_s(intHex, "%X", address), luaL_error(L, "%s address: 0x%s", errorReport, intHex));

extern char intHex[4 + 1];
extern char errorReport[1000];
int errorFilterAndReporter(unsigned int code, struct _EXCEPTION_POINTERS* ep);