
#include "framework.h"
#include <iostream>
#include <sstream>
#include <set>
#include <map>
#include <assert.h>
#include <vector>

#include "RuntimePatchingSystem.h"
#include "CodeFunctions.h"
#include "MemoryFunctions.h"
#include "UtilityFunctions.h"
#include "LibraryFunctions.h"


static int l_my_print(lua_State* L) {
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	for (i = 1; i <= n; i++) {  /* for each argument */
		size_t l;
		const char* s = luaL_tolstring(L, i, &l);  /* convert it to string */
		if (i > 1)  /* not the first element? */
			lua_writestring("\t", 1);  /* add a tab before it */
		lua_writestring(s, l);  /* print it */
		lua_pop(L, 1);  /* pop result */
	}
	lua_writeline();
	return 0;
}

static const struct luaL_Reg printlib[] = {
  {"print", l_my_print},
  {NULL, NULL} /* end of array */
};

const struct luaL_Reg RPS_LIB[] = {
	{"hookCode", luaHookCode},
	{"callOriginal", luaCallMachineCode},
	{"exposeCode", luaExposeCode},
	{"detourCode", luaDetourCode},
	{"allocate", luaAllocate},
	{"deallocate", luaDeallocate},
	{"allocateCode", luaAllocateRWE},
	{"deallocateCode", luaDeallocateRWE},

	{"loadLibraryA", luaLoadLibraryA},
	{"getLibraryProcAddressA", luaGetLibraryProcAddressA},
	{"getProcAddress", luaGetProcAddress},

	{"readByte", luaReadByte},
	{"readSmallInteger", luaReadSmallInteger},
	{"readInteger", luaReadInteger},
	{"readString", luaReadString},
	{"readBytes", luaReadBytes},

	{"writeByte", luaWriteByte},
	{"writeSmallInteger", luaWriteSmallInteger},
	{"writeInteger", luaWriteInteger},
	{"writeBytes", luaWriteBytes},
	{"writeString", luaWriteString},
	{"writeCode", luaWriteCode},

	{"copyMemory", luaMemCpy},

	{"registerString", registerString},

	{"scanForAOB", luaScanForAOB},
	{NULL, NULL} /* end of array */
};


RUNTIMEPATCHINGSYSTEM_API void RPS_initializePrintRedirect(lua_State* L) {
	lua_pushglobaltable(L);
	luaL_setfuncs(L, printlib, 0);
	lua_pop(L, 1);
}

RUNTIMEPATCHINGSYSTEM_API void RPS_initializePrintRedirect() {
	return RPS_initializePrintRedirect(L);
}

RUNTIMEPATCHINGSYSTEM_API void RPS_initializeLuaAPI(lua_State* L, std::string apiNamespace) {
	if (apiNamespace == "_G" || apiNamespace == "global") {
		lua_pushglobaltable(L);
		luaL_setfuncs(L, RPS_LIB, 0);
		lua_pop(L, 1);
	}
	else if (apiNamespace.empty() || apiNamespace.size() == 0) {
		luaL_newlib(L, RPS_LIB); // the table is left intentionally on the stack.
	}
	else {
		lua_pushglobaltable(L);
		luaL_newlib(L, RPS_LIB);
		lua_setfield(L, -1, apiNamespace.c_str());
		lua_pop(L, 1);
	}
}

RUNTIMEPATCHINGSYSTEM_API void RPS_initializeLuaAPI(std::string apiNamespace) {
	return RPS_initializeLuaAPI(L, apiNamespace);
}

RUNTIMEPATCHINGSYSTEM_API void RPS_initializeLuaAPI(lua_State* L) {
	return RPS_initializeLuaAPI(L, "global");
}

RUNTIMEPATCHINGSYSTEM_API void RPS_initializeLuaAPI() {
	return RPS_initializeLuaAPI(L);
}

RUNTIMEPATCHINGSYSTEM_API void RPS_setupPackagePath(lua_State* L, std::string packagePath) {
	lua_getglobal(L, "package");
	lua_pushstring(L, "path");
	lua_pushstring(L, packagePath.c_str());
	lua_settable(L, -3);
	lua_pop(L, 1);
}

RUNTIMEPATCHINGSYSTEM_API void RPS_setupPackagePath(std::string packagePath) {
	return RPS_setupPackagePath(L, packagePath);
}

RUNTIMEPATCHINGSYSTEM_API void RPS_setupPackageCPath(lua_State* L, std::string packageCPath) {
	lua_getglobal(L, "package");
	lua_pushstring(L, "cpath");
	lua_pushstring(L, packageCPath.c_str());
	lua_settable(L, -3);
	lua_pop(L, 1);
}

RUNTIMEPATCHINGSYSTEM_API void RPS_setupPackageCPath(std::string packageCPath) {
	return RPS_setupPackageCPath(L, packageCPath);
}

RUNTIMEPATCHINGSYSTEM_API void RPS_runBootstrapFile(lua_State* L, std::string bootstrapFilePath) {
	int stackSize = lua_gettop(L);

	int r = luaL_dofile(L, bootstrapFilePath.c_str());

	if (r == LUA_OK) {
		//std::cout << "[LUA]: loaded LUA API." << std::endl;

		lua_pop(L, lua_gettop(L) - stackSize);
	}
	else {
		size_t length;
		const char* errormsg = lua_tolstring(L, -1, &length);
		if (errormsg != NULL) {
			std::cout << "[RPS]: failed to execute lua file: " << errormsg << std::endl;
			lua_pop(L, 1); // pop off the error message;
		}
		else {
			std::cout << "[RPS]: failed to execute lua file: lua error code: " << r << std::endl;
		}

	}
}

RUNTIMEPATCHINGSYSTEM_API void RPS_runBootstrapFile(std::string bootstrapFilePath) {
	return RPS_runBootstrapFile(L, bootstrapFilePath);
}

RUNTIMEPATCHINGSYSTEM_API void RPS_initializeLua() {
	L = luaL_newstate();
}

RUNTIMEPATCHINGSYSTEM_API void RPS_initializeLuaOpenLibs() {
	luaL_openlibs(L);
}

RUNTIMEPATCHINGSYSTEM_API void RPS_initializeLuaOpenBase() {
	luaL_requiref(L, "base", luaopen_base, true);
}

RUNTIMEPATCHINGSYSTEM_API void RPS_initialize(std::string bootstrapFilePath, bool initializePrintRedirect) {
	RPS_initializeLua();

	RPS_initializeLuaOpenLibs();

	if (initializePrintRedirect) RPS_initializePrintRedirect(L);

	RPS_initializeLuaAPI(L, "global");


	RPS_runBootstrapFile(L, bootstrapFilePath);
}

RUNTIMEPATCHINGSYSTEM_API void RPS_initialize(std::string bootstrapFilePath) {
	return RPS_initialize(bootstrapFilePath, true);
}

RUNTIMEPATCHINGSYSTEM_API void RPS_deinitialize() {
	lua_close(L);
}

RUNTIMEPATCHINGSYSTEM_API void RPS_executeSnippet(std::string code) {
	int before = lua_gettop(L);
	int r = luaL_dostring(L, code.c_str());
	if (r == LUA_OK) {
		int after = lua_gettop(L);
		if (after - before > 0) {
			for (int i = before; i < after; i++) {  /* for each argument */
				size_t l;
				const char* s = luaL_tolstring(L, i, &l);  /* convert it to string */
				if (i > 1)  /* not the first element? */
					lua_writestring("\t", 1);  /* add a tab before it */
				lua_writestring(s, l);  /* print it */
				lua_pop(L, 1);  /* pop result */
			}
			lua_writeline();
		}
		lua_pop(L, after - before);
	}
	else {
		std::string errormsg = lua_tostring(L, -1);
		std::cout << "[RPS]: error in lua snippet: " << errormsg << std::endl;
		lua_pop(L, 1); // pop off the error message;
	}
}

RUNTIMEPATCHINGSYSTEM_API int RPS_getCurrentStackSize() {
	return lua_gettop(L);
}

RUNTIMEPATCHINGSYSTEM_API lua_State* RPS_getLuaState() {
	return L;
}

RUNTIMEPATCHINGSYSTEM_API void RPS_setLuaState(lua_State* value) {
	L = value;
}

extern "C" RUNTIMEPATCHINGSYSTEM_API int luaopen_RPS(lua_State * L) {

	luaL_newlib(L, RPS_LIB);

	return 1;
}
