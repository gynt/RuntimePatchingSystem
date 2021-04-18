#include "framework.h"
#include "RuntimePatchingLibrary.h"

RUNTIMEPATCHINGLIBRARY_API void RPL_initialize(std::string bootstrapFilePath, std::string packagePath) {
	return LuaAPI::initialize(bootstrapFilePath, packagePath);
}

RUNTIMEPATCHINGLIBRARY_API void RPL_deinitialize() {
	return LuaAPI::deinitialize();
}

RUNTIMEPATCHINGLIBRARY_API void RPL_executeSnippet(std::string code) {
	return LuaAPI::executeSnippet(code);
}

RUNTIMEPATCHINGLIBRARY_API int RPL_getCurrentStackSize() {
	return LuaAPI::getCurrentStackSize();
}

RUNTIMEPATCHINGLIBRARY_API lua_State* RPL_getLuaState() {
	return LuaAPI::getLuaState();
}

RUNTIMEPATCHINGLIBRARY_API void RPL_initializeLuaAPI(lua_State* L, bool includePrintRedirect) {
	return LuaAPI::initializeLuaAPI(L, includePrintRedirect);
}

RUNTIMEPATCHINGLIBRARY_API void setupPackagePath(lua_State* L, std::string packagePath) {
	return LuaAPI::setupPackagePath(L, packagePath);
}

RUNTIMEPATCHINGLIBRARY_API void runBootstrapFile(lua_State* L, std::string bootstrapFilePath) {
	return LuaAPI::runBootstrapFile(L, bootstrapFilePath);
}

RUNTIMEPATCHINGLIBRARY_API void initializeLua() {
	return LuaAPI::initializeLua();
}