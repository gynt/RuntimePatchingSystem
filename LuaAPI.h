#pragma once

// LUA
#include "lua.hpp"
#include <string>

namespace LuaAPI {

	void initialize(std::string bootstrapFilePath, std::string packagePath);
	void deinitialize();
	void executeSnippet(std::string code);
	int getCurrentStackSize();
	lua_State* getLuaState();
}
