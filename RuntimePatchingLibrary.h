#ifdef RUNTIMEPATCHINGLIBRARY_EXPORTS
#define RUNTIMEPATCHINGLIBRARY_API __declspec(dllexport)
#else
#define RUNTIMEPATCHINGLIBRARY_API __declspec(dllimport)
#endif

#include <string>
#include "lua.hpp"

RUNTIMEPATCHINGLIBRARY_API void RPL_initialize(std::string bootstrapFilePath, std::string packagePath);

RUNTIMEPATCHINGLIBRARY_API void RPL_deinitialize();

RUNTIMEPATCHINGLIBRARY_API void RPL_executeSnippet(std::string code);

RUNTIMEPATCHINGLIBRARY_API int RPL_getCurrentStackSize();

RUNTIMEPATCHINGLIBRARY_API lua_State* RPL_getLuaState();

RUNTIMEPATCHINGLIBRARY_API void RPL_initializeLuaAPI(lua_State* L, bool includePrintRedirect);


RUNTIMEPATCHINGLIBRARY_API void RPL_initializeLuaAPI(lua_State* L, bool includePrintRedirect);

RUNTIMEPATCHINGLIBRARY_API void RPL_setupPackagePath(lua_State* L, std::string packagePath);

RUNTIMEPATCHINGLIBRARY_API void RPL_runBootstrapFile(lua_State* L, std::string bootstrapFilePath);

RUNTIMEPATCHINGLIBRARY_API void RPL_initializeLua();