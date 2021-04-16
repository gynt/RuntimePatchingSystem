// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the RUNTIMEPATCHINGLIBRARY_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// RUNTIMEPATCHINGLIBRARY_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef RUNTIMEPATCHINGLIBRARY_EXPORTS
#define RUNTIMEPATCHINGLIBRARY_API __declspec(dllexport)
#else
#define RUNTIMEPATCHINGLIBRARY_API __declspec(dllimport)
#endif

#include <string>
#include "LuaAPI.h"

//// This class is exported from the dll
//class RUNTIMEPATCHINGLIBRARY_API CRuntimePatchingLibrary {
//public:
//	CRuntimePatchingLibrary(void);
//	// TODO: add your methods here.
//};
//
//extern RUNTIMEPATCHINGLIBRARY_API int nRuntimePatchingLibrary;
//
//RUNTIMEPATCHINGLIBRARY_API int fnRuntimePatchingLibrary(void);

RUNTIMEPATCHINGLIBRARY_API void RPL_initialize();

RUNTIMEPATCHINGLIBRARY_API void RPL_deinitialize();

RUNTIMEPATCHINGLIBRARY_API void RPL_executeSnippet(std::string code);

RUNTIMEPATCHINGLIBRARY_API int RPL_getCurrentStackSize();

RUNTIMEPATCHINGLIBRARY_API lua_State* RPL_getLuaState();
