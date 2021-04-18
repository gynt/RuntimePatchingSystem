#ifdef RUNTIMEPATCHINGLIBRARY_EXPORTS
#define RUNTIMEPATCHINGLIBRARY_API __declspec(dllexport)
#else
#define RUNTIMEPATCHINGLIBRARY_API __declspec(dllimport)
#endif

#include <string>
#include "LuaAPI.h"

RUNTIMEPATCHINGLIBRARY_API void RPL_initialize(std::string bootstrapFilePath, std::string packagePath);

RUNTIMEPATCHINGLIBRARY_API void RPL_deinitialize();

RUNTIMEPATCHINGLIBRARY_API void RPL_executeSnippet(std::string code);

RUNTIMEPATCHINGLIBRARY_API int RPL_getCurrentStackSize();

RUNTIMEPATCHINGLIBRARY_API lua_State* RPL_getLuaState();
