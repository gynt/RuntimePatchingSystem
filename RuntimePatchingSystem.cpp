#include "framework.h"
#include "RuntimePatchingSystem.h"
#include "LuaAPI.h"

RUNTIMEPATCHINGSYSTEM_API void RPL_initialize(std::string bootstrapFilePath, std::string packagePath) {
	return LuaAPI::initialize(bootstrapFilePath, packagePath);
}

RUNTIMEPATCHINGSYSTEM_API void RPL_deinitialize() {
	return LuaAPI::deinitialize();
}

RUNTIMEPATCHINGSYSTEM_API void RPL_executeSnippet(std::string code) {
	return LuaAPI::executeSnippet(code);
}

RUNTIMEPATCHINGSYSTEM_API int RPL_getCurrentStackSize() {
	return LuaAPI::getCurrentStackSize();
}

RUNTIMEPATCHINGSYSTEM_API lua_State* RPL_getLuaState() {
	return LuaAPI::getLuaState();
}

RUNTIMEPATCHINGSYSTEM_API void RPL_initializeLuaAPI(lua_State* L, bool includePrintRedirect) {
	return LuaAPI::initializeLuaAPI(L, includePrintRedirect);
}

RUNTIMEPATCHINGSYSTEM_API void RPL_setupPackagePath(lua_State* L, std::string packagePath) {
	return LuaAPI::setupPackagePath(L, packagePath);
}

RUNTIMEPATCHINGSYSTEM_API void RPL_runBootstrapFile(lua_State* L, std::string bootstrapFilePath) {
	return LuaAPI::runBootstrapFile(L, bootstrapFilePath);
}

RUNTIMEPATCHINGSYSTEM_API void RPL_initializeLua() {
	return LuaAPI::initializeLua();
}