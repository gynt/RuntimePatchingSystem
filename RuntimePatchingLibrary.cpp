// RuntimePatchingLibrary.cpp : Defines the exported functions for the DLL.
//

#include "framework.h"
#include "RuntimePatchingLibrary.h"


//// This is an example of an exported variable
//RUNTIMEPATCHINGLIBRARY_API int nRuntimePatchingLibrary=0;
//
//// This is an example of an exported function.
//RUNTIMEPATCHINGLIBRARY_API int fnRuntimePatchingLibrary(void)
//{
//    return 0;
//}
//
//// This is the constructor of a class that has been exported.
//CRuntimePatchingLibrary::CRuntimePatchingLibrary()
//{
//    return;
//}

RUNTIMEPATCHINGLIBRARY_API void RLP_initialize() {
	return LuaAPI::initialize();
}

RUNTIMEPATCHINGLIBRARY_API void RLP_deinitialize() {
	return LuaAPI::deinitialize();
}

RUNTIMEPATCHINGLIBRARY_API void RLP_executeSnippet(std::string code) {
	return LuaAPI::executeSnippet(code);
}

RUNTIMEPATCHINGLIBRARY_API int getCurrentStackSize() {
	return LuaAPI::getCurrentStackSize();
}

RUNTIMEPATCHINGLIBRARY_API lua_State* getLuaState() {
	return LuaAPI::getLuaState();
}