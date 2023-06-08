#pragma once

#include <lua.hpp>


int luaLoadLibraryA(lua_State* L);
int luaGetLibraryProcAddressA(lua_State* L);
int luaGetProcAddress(lua_State* L);
