#include "framework.h"
#include "RuntimePatchingSystem.h"
#include "LuaAPI.h"
#include "AOB.h"

RUNTIMEPATCHINGSYSTEM_API void RPS_initialize(std::string bootstrapFilePath, std::string packagePath) {
	return LuaAPI::initialize(bootstrapFilePath, packagePath);
}

RUNTIMEPATCHINGSYSTEM_API void RPS_deinitialize() {
	return LuaAPI::deinitialize();
}

RUNTIMEPATCHINGSYSTEM_API void RPS_executeSnippet(std::string code) {
	return LuaAPI::executeSnippet(code);
}

RUNTIMEPATCHINGSYSTEM_API int RPS_getCurrentStackSize() {
	return LuaAPI::getCurrentStackSize();
}

RUNTIMEPATCHINGSYSTEM_API lua_State* RPS_getLuaState() {
	return LuaAPI::getLuaState();
}

RUNTIMEPATCHINGSYSTEM_API void RPS_initializeLuaAPI(lua_State* L, bool includePrintRedirect) {
	return LuaAPI::initializeLuaAPI(L, includePrintRedirect);
}

RUNTIMEPATCHINGSYSTEM_API void RPS_setupPackagePath(lua_State* L, std::string packagePath) {
	return LuaAPI::setupPackagePath(L, packagePath);
}

RUNTIMEPATCHINGSYSTEM_API void RPS_runBootstrapFile(lua_State* L, std::string bootstrapFilePath) {
	return LuaAPI::runBootstrapFile(L, bootstrapFilePath);
}

RUNTIMEPATCHINGSYSTEM_API void RPS_initializeLua() {
	return LuaAPI::initializeLua();
}

RUNTIMEPATCHINGSYSTEM_API DWORD RPS_findAOB(std::string aob_hex) {
	return AOB::Find(aob_hex);
}

RUNTIMEPATCHINGSYSTEM_API DWORD RPS_findAOB(std::string aob_hex, DWORD min, DWORD max) {
	return AOB::FindInRange(aob_hex, min, max);
}

RUNTIMEPATCHINGSYSTEM_API DWORD RPS_findAOB(char* content, char* mask, DWORD min, DWORD max) {
	return AOB::Scan(content, mask, min, max);
}