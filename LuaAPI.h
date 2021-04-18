#pragma once

// LUA
#include "lua.hpp"
#include <string>

namespace LuaAPI {

	void initializeLuaAPI(lua_State* L, bool includePrintRedirect);
	void initialize(std::string bootstrapFilePath, std::string packagePath);
	void deinitialize();
	void executeSnippet(std::string code);
	int getCurrentStackSize();
	lua_State* getLuaState();

	void setupPackagePath(lua_State* L, std::string packagePath);

	void runBootstrapFile(lua_State* L, std::string bootstrapFilePath);

	void initializeLua();
}
