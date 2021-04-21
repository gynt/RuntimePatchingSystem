#ifdef RUNTIMEPATCHINGLIBRARY_EXPORTS
#define RUNTIMEPATCHINGSYSTEM_API __declspec(dllexport)
#else
#define RUNTIMEPATCHINGSYSTEM_API __declspec(dllimport)
#endif

#include <string>
#include "lua.hpp"

RUNTIMEPATCHINGSYSTEM_API void RPL_initialize(std::string bootstrapFilePath, std::string packagePath);

RUNTIMEPATCHINGSYSTEM_API void RPL_deinitialize();

RUNTIMEPATCHINGSYSTEM_API void RPL_executeSnippet(std::string code);

RUNTIMEPATCHINGSYSTEM_API int RPL_getCurrentStackSize();

RUNTIMEPATCHINGSYSTEM_API lua_State* RPL_getLuaState();

RUNTIMEPATCHINGSYSTEM_API void RPL_initializeLuaAPI(lua_State* L, bool includePrintRedirect);


RUNTIMEPATCHINGSYSTEM_API void RPL_initializeLuaAPI(lua_State* L, bool includePrintRedirect);

RUNTIMEPATCHINGSYSTEM_API void RPL_setupPackagePath(lua_State* L, std::string packagePath);

RUNTIMEPATCHINGSYSTEM_API void RPL_runBootstrapFile(lua_State* L, std::string bootstrapFilePath);

RUNTIMEPATCHINGSYSTEM_API void RPL_initializeLua();