#pragma once

#ifdef RUNTIMEPATCHINGLIBRARY_EXPORTS
#define RUNTIMEPATCHINGSYSTEM_API __declspec(dllexport)
#else
#define RUNTIMEPATCHINGSYSTEM_API __declspec(dllimport)
#endif

// LUA
#include "lua.hpp"
#include <string>


RUNTIMEPATCHINGSYSTEM_API extern const struct luaL_Reg RPS_LIB[];

RUNTIMEPATCHINGSYSTEM_API void RPS_initializeLuaAPI(lua_State* L, std::string apiNamespace);

RUNTIMEPATCHINGSYSTEM_API void RPS_initialize(std::string bootstrapFilePath, std::string packagePath, bool initializePrintRedirect);

RUNTIMEPATCHINGSYSTEM_API void RPS_deinitialize();

RUNTIMEPATCHINGSYSTEM_API void RPS_executeSnippet(std::string code);

RUNTIMEPATCHINGSYSTEM_API int RPS_getCurrentStackSize();

RUNTIMEPATCHINGSYSTEM_API lua_State* RPS_getLuaState();

RUNTIMEPATCHINGSYSTEM_API void RPS_setupPackagePath(lua_State* L, std::string packagePath);

RUNTIMEPATCHINGSYSTEM_API void RPS_runBootstrapFile(lua_State* L, std::string bootstrapFilePath);

RUNTIMEPATCHINGSYSTEM_API void RPS_initializeLua();

RUNTIMEPATCHINGSYSTEM_API void RPS_initializePrintRedirect(lua_State* L);

RUNTIMEPATCHINGSYSTEM_API bool RPS_initializeCodeHeap();

