/*******************************************************************
 * \file   RuntimePatchingSystem.h
 * \brief  Documentation for the API functions
 * 
 * \author gynt
 * \date   July 2021
 *********************************************************************/

/**
 * @mainpage Runtime Patching System overview.
 * 
 * @section Overview
 */


#pragma once

#ifdef RUNTIMEPATCHINGLIBRARY_EXPORTS
#define RUNTIMEPATCHINGSYSTEM_API __declspec(dllexport)
#else
#define RUNTIMEPATCHINGSYSTEM_API __declspec(dllimport)
#endif

#include "lua.hpp"
#include <string>

/**
 * Initializes a lua environment. Registers RPS lua functions in that environment.
 * @param bootstrapFilePath path of the bootstrap lua file.
 */
RUNTIMEPATCHINGSYSTEM_API void RPS_initialize(std::string bootstrapFilePath);

/**
 * Initialize a lua state and run the bootstrap file. The lua RPS functions are put in the global namespace. Optional redirection of `print` output to std::cout
 */
RUNTIMEPATCHINGSYSTEM_API void RPS_initialize(std::string bootstrapFilePath, bool initializePrintRedirect);

/**
 * Initializes the lua state.
 */
RUNTIMEPATCHINGSYSTEM_API void RPS_initializeLua();

/**
 * Sets up the `print` redirect to `std::cout`.
 */
RUNTIMEPATCHINGSYSTEM_API void RPS_initializePrintRedirect(lua_State* L);
RUNTIMEPATCHINGSYSTEM_API void RPS_initializePrintRedirect();

/**
 * Registers the lua api in the global namespace.
 */
RUNTIMEPATCHINGSYSTEM_API void RPS_initializeLuaAPI(lua_State* L);
RUNTIMEPATCHINGSYSTEM_API void RPS_initializeLuaAPI();

/**
 * Load all common libraries, including base.
 */
RUNTIMEPATCHINGSYSTEM_API void RPS_initializeLuaOpenLibs();

/**
 * Load the base library.
 */
RUNTIMEPATCHINGSYSTEM_API void RPS_initializeLuaOpenBase();

/**
 * Registers the lua api in the given namespace. A table with the functions with the given name is created in the global namespace
 * @param apiNamespace if NULL or empty, the table is left on the lua stack. If "global" the functions are registered in the global namespace.
 */
RUNTIMEPATCHINGSYSTEM_API void RPS_initializeLuaAPI(lua_State* L, std::string apiNamespace);
RUNTIMEPATCHINGSYSTEM_API void RPS_initializeLuaAPI(std::string apiNamespace);
RUNTIMEPATCHINGSYSTEM_API void RPS_initializeLuaAPI();

/**
 * The functions to register in the global namespace.
 */
RUNTIMEPATCHINGSYSTEM_API extern const struct luaL_Reg RPS_LIB[];

/**
 * Initializes the heap memory used by the lua code functions to store executable code.
 */
RUNTIMEPATCHINGSYSTEM_API bool RPS_initializeCodeHeap();

/**
 * Deinitialize the lua api. Destroys the lua state and destroys the code heap.
 */
RUNTIMEPATCHINGSYSTEM_API void RPS_deinitialize();

/**
 * Execute a snippet of code in the lua state.
 */
RUNTIMEPATCHINGSYSTEM_API void RPS_executeSnippet(std::string code);

/**
 * Get the lua state.
 */
RUNTIMEPATCHINGSYSTEM_API lua_State* RPS_getLuaState();

/**
 * Set the lua state that is used internally.
 */
RUNTIMEPATCHINGSYSTEM_API void RPS_setLuaState(lua_State* value);

/**
 * Get the amount of objects on the lua stack.
 */
RUNTIMEPATCHINGSYSTEM_API int RPS_getCurrentStackSize();

/**
 * Setup the package path by changing the `package.path` lua variable.
 */
RUNTIMEPATCHINGSYSTEM_API void RPS_setupPackagePath(lua_State* L, std::string packagePath);
RUNTIMEPATCHINGSYSTEM_API void RPS_setupPackagePath(std::string packagePath);

/**
 * Setup the package cpath by changing the `package.cpath` lua variable..
 */
RUNTIMEPATCHINGSYSTEM_API void RPS_setupPackageCPath(lua_State* L, std::string packageCPath);
RUNTIMEPATCHINGSYSTEM_API void RPS_setupPackageCPath(std::string packageCPath);

/**
 * Run a (bootstrap) file.
 */
RUNTIMEPATCHINGSYSTEM_API void RPS_runBootstrapFile(lua_State* L, std::string bootstrapFilePath);
RUNTIMEPATCHINGSYSTEM_API void RPS_runBootstrapFile(std::string bootstrapFilePath);







