#pragma once

#include "framework.h"
#include "lua.hpp"
#include <string>
#include <assert.h>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include "AOB.h"
#include "UtilityFunctions.h"

extern HANDLE codeHeap;

int luaWriteCode(lua_State* L);

int luaAllocateRWE(lua_State* L);

int luaScanForAOB(lua_State* L);

int luaDetourCode(lua_State* L);

int luaExposeCode(lua_State* L);

int luaHookCode(lua_State* L);

int luaCallMachineCode(lua_State* L);

