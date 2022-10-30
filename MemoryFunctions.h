#pragma once

#include "framework.h"
#include "lua.hpp"
#include <string>
#include "UtilityFunctions.h"
#include <set>
#include <map>

int registerString(lua_State* L);

int luaReadByte(lua_State* L);

int luaReadSmallInteger(lua_State* L);

int luaReadInteger(lua_State* L);

int luaReadString(lua_State* L);

int luaReadBytes(lua_State* L);

int luaWriteByte(lua_State* L);

int luaWriteSmallInteger(lua_State* L);

int luaWriteInteger(lua_State* L);

int luaWriteBytes(lua_State* L);

int luaWriteString(lua_State* L);

int luaMemCpy(lua_State* L);

int luaAllocate(lua_State* L);

int luaDeallocate(lua_State* L);
