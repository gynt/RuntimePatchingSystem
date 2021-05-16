
#include "framework.h"
#include <iostream>
#include <sstream>
#include <set>
#include <map>
#include <assert.h>
#include <vector>
#include "AOB.h"

#include "LuaAPI.h"
#include "CodeFunctions.h"
#include "MemoryFunctions.h"
#include "UtilityFunctions.h"

namespace LuaAPI {



	void LuaLandingFromCpp();
	void detourLandingFunction();

	int CALLING_CONV_CALLER = 0; //add esp, 4*argumentCount
	int CALLING_CONV_THISCALL = 1; //mov ecx, address
	int CALLING_CONV_STDCALL = 2; //special ret 0x, no this parameter
	//int CALLING_CONV_FASTCALL = 3; ?

	HANDLE codeHeap;

	bool DoCreateCallHook(DWORD from_address, DWORD to_address, int hookSize, DWORD& newFunctionLocation) {
		constexpr INT8 NOP = (INT8)0x90;
		constexpr INT8 JMP = (INT8)0xE9;
		constexpr INT8 CALL = (INT8)0xE8;

		int size = hookSize;
		if (size < 5) return FALSE;

		BYTE* fun_o_ptr = (BYTE*)from_address;
		BYTE* fun_h_ptr = (BYTE*)to_address;

		// create gateway
		BYTE* gateway = (BYTE*)VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		memcpy_s(gateway, size, fun_o_ptr, size);
		uintptr_t gatewayRelAddress = fun_o_ptr - gateway - 5;

		*(gateway + size) = JMP;
		*(uintptr_t*)((uintptr_t)gateway + size + 1) = gatewayRelAddress;

		// detour

		DWORD oldProtect;
		VirtualProtect(fun_o_ptr, size, PAGE_EXECUTE_READWRITE, &oldProtect);

		memset(fun_o_ptr, NOP, size); // needs to be done, otherwise this confuses the CE disassmbler

		uintptr_t relAddress = fun_h_ptr - fun_o_ptr - 5;

		*fun_o_ptr = CALL;
		*(uintptr_t*)(fun_o_ptr + 1) = relAddress;

		VirtualProtect(fun_o_ptr, size, oldProtect, &oldProtect);

		// point original function to gateway
		newFunctionLocation = (DWORD)& gateway[0];

		return true;
	}

	int luaCallMachineCode(lua_State* L);

	lua_State* L;

	class LuaHook {
	public:
		DWORD address;
		int hookSize;
		int callingConvention;
		int argumentsCount;
		DWORD newOriginalFunctionLocation;
		std::string luaHookFunctionName;
		std::string luaOriginalFunctionName;
		DWORD thisValue;
		int luaTableRef;
		int luaHookFunctionRef;

		LuaHook(DWORD addr, int hookSize, int callingConv, int argCount, std::string luaHook, std::string luaOriginal) {
			this->address = addr;
			this->hookSize = hookSize;
			this->callingConvention = callingConv;
			this->argumentsCount = argCount - (int)(callingConv == 1); // To keep it clear for the lua users...
			this->luaHookFunctionName = luaHook;
			this->luaOriginalFunctionName = luaOriginal;
			this->luaTableRef = -1;
			this->luaHookFunctionRef = -1;
		}

		bool CreateCallHook() {
			return DoCreateCallHook(this->address, (DWORD)LuaLandingFromCpp, this->hookSize, this->newOriginalFunctionLocation);
		}

		void setTableRef(lua_State* L) {
			/** Pops a value from the stack and registers it in the registry. */
			this->luaTableRef = luaL_ref(L, LUA_REGISTRYINDEX);
		}

		void setHookFunctionRef(lua_State* L) {
			assert(this->luaTableRef != -1);

			// Put the table at the top of the stack.
			lua_rawgeti(L, LUA_REGISTRYINDEX, this->luaTableRef);

			// Put the key on top of the stack.
			lua_pushstring(L, this->luaHookFunctionName.c_str());

			// Puts the value on top of the stack
			lua_gettable(L, -2);

			if (lua_isfunction(L, -1)) {
				this->luaHookFunctionRef = luaL_ref(L, -1);
			}
			else {
				throw "cannot set hookfunctionref because there is no function with the name: " + this->luaHookFunctionName;
			}
		}

		void registerOriginalFunctionInLua() {

			if (this->luaTableRef == -1) {
				lua_pushglobaltable(L);
			}
			else {
				// Put the table on the top of the stack.
				lua_rawgeti(L, LUA_REGISTRYINDEX, this->luaTableRef);
			}

			// Put the key on the top of the stack
			lua_pushstring(L, this->luaOriginalFunctionName.c_str());

			lua_pushinteger(L, this->address);
			lua_pushcclosure(L, &luaCallMachineCode, 1);

			lua_settable(L, -3);
			lua_pop(L, 1); // Pop the table

		}
	};

	std::map<DWORD, std::shared_ptr<LuaHook>> hookMapping;

	/**
		These variables are here to keep the raw assembly code easy and avoid this-parameter code.
	*/
	char luaHookedFunctionName[100];
	DWORD newOriginalFunctionLocation;
	DWORD functionLocation;
	int luaHookedFunctionArgCount;
	int luaErrorLevel;
	std::string luaErrorMsg;
	int luaCallingConvention;
	DWORD currentECXValue;
	int luaHookedFunctionTableReference;

	// lua calls this function as: exposeCode(luaOriginal = "callback", address = 0xDEADBEEF, argumentCount = 3, callingConvention = 0)
	//  or lua calls this function as: exposeCode(luaOriginal = "callback", address = 0xDEADBEEF, argumentCount = 3, callingConvention = 0, env = table)
	int luaExposeCode(lua_State* L) {

		if (lua_gettop(L) < 4 || lua_gettop(L) > 5) {
			return luaL_error(L, "expecting exactly 4 or 5 arguments");
		}

		std::string luaOriginal = lua_tostring(L, 1);
		DWORD address = lua_tointeger(L, 2);
		int argumentCount = lua_tointeger(L, 3);
		int callingConvention = lua_tointeger(L, 4);

		if (argumentCount > 8) {
			return luaL_error(L, "too many arguments specified, max is 8: " + argumentCount);
		}

		hookMapping.insert(std::pair<DWORD, std::shared_ptr<LuaHook>>(address, std::make_shared<LuaHook>(address, 0, callingConvention, argumentCount, "", luaOriginal)));
		
		if (lua_gettop(L) == 5) {
			// a custom env was specified, push it again to make it at the top of the stack.
			lua_pushvalue(L, 5);
			
			// Pops off the table from the stack.
			hookMapping[address]->setTableRef(L);
		}
		else {
			// assume the env is the global env
		}
		
		hookMapping[address]->newOriginalFunctionLocation = address;
		hookMapping[address]->registerOriginalFunctionInLua();

		return 0;
	}

	// lua calls this function as: hookCode(luaHook = "callbackTest", luaOriginal = "callback", address = 0xDEADBEEF, argumentCount = 3, callingConvention = 0, hookSize = 5)
	// or as: hookCode(luaHook = "callbackTest", luaOriginal = "callback", address = 0xDEADBEEF, argumentCount = 3, callingConvention = 0, hookSize = 5, env)
	int luaHookCode(lua_State* L) {

		if (lua_gettop(L) < 6 || lua_gettop(L) > 7) {
			return luaL_error(L, "expecting exactly 6 or 7 arguments");
		}

		std::string luaHook = lua_tostring(L, 1);
		std::string luaOriginal = lua_tostring(L, 2);
		DWORD address = lua_tointeger(L, 3);
		int argumentCount = lua_tointeger(L, 4);
		int callingConvention = lua_tointeger(L, 5);
		int hookSize = lua_tointeger(L, 6);

		if (argumentCount > 8) {
			return luaL_error(L, "too many arguments specified, max is 8: " + argumentCount);
		}

		hookMapping.insert(std::pair<DWORD, std::shared_ptr<LuaHook>>(address, std::make_shared<LuaHook>(address, hookSize, callingConvention, argumentCount, luaHook, luaOriginal)));
		if (lua_gettop(L) == 7) {
			// a custom env was specified, push it again to make it at the top of the stack.
			lua_pushvalue(L, 7);

			// Pops off the table from the stack.
			hookMapping[address]->setTableRef(L);
		}
		else {
			// assume the env is the global env
	
		}
		hookMapping[address]->CreateCallHook();
		hookMapping[address]->registerOriginalFunctionInLua();

		return 1;
	}


	DWORD __stdcall executeLuaHook(unsigned long* args) {

		if (luaHookedFunctionTableReference == -1) {
			lua_getglobal(L, luaHookedFunctionName);
		}
		else {
			lua_rawgeti(L, LUA_REGISTRYINDEX, luaHookedFunctionTableReference);
			lua_getfield(L, -1, luaHookedFunctionName);
			lua_remove(L, lua_absindex(L, -2)); // remove the table
		}

		if (lua_isfunction(L, -1)) {
			int totalArgCount = luaHookedFunctionArgCount;
			if (luaCallingConvention == 1) {
				lua_pushnumber(L, currentECXValue);
				totalArgCount += 1;
			}
			for (int i = 0; i < luaHookedFunctionArgCount; i++) {
				lua_pushnumber(L, args[i]);
			}
			if (lua_pcall(L, totalArgCount, 1, 0) == LUA_OK) {
				luaErrorLevel = 0;
				luaErrorMsg = "";
				int ret = (DWORD)lua_tonumber(L, -1);
				lua_pop(L, 1); // pop off the return value;
				return ret;
			}
			else {
				luaErrorLevel = 1;
				luaErrorMsg = lua_tostring(L, -1);
				lua_pop(L, 1); // pop off the error message;
			}
		}
		else {
			lua_pop(L, 1); // I think we need this pop, because the getglobal does a push that would otherwise be popped by pcall.
			luaErrorLevel = 2;
			luaErrorMsg = std::string(luaHookedFunctionName) + " is not a function";
		}

		std::cout << "[LUA API]: " << std::string(luaErrorMsg) << std::endl;
		return 0;
	}

	void __stdcall SetLuaHookedFunctionParameters(DWORD origin, DWORD liveECXValue) {
		std::map<DWORD, std::shared_ptr<LuaHook>>::iterator it;

		it = hookMapping.find(origin);
		if (it != hookMapping.end()) {
			std::shared_ptr<LuaHook> value = it->second;
			memset(luaHookedFunctionName, 0, 100);
			memcpy(luaHookedFunctionName, value->luaHookFunctionName.c_str(), value->luaHookFunctionName.size());
			luaHookedFunctionArgCount = value->argumentsCount;
			luaCallingConvention = value->callingConvention;
			newOriginalFunctionLocation = value->newOriginalFunctionLocation;
			functionLocation = value->address;
			currentECXValue = liveECXValue;
			luaHookedFunctionTableReference = value->luaTableRef;
		}
		else {

		}

	}

	DWORD fakeStack[20];

	// The user has called the luaOriginalFunctionName
	int luaCallMachineCode(lua_State* L) {
		DWORD address = lua_tointeger(L, lua_upvalueindex(1)); //lua_upvalueindex(1)

		SetLuaHookedFunctionParameters(address, 0);

		if (luaCallingConvention == 1) {
			int totalArgCount = luaHookedFunctionArgCount + 1;  // ecx is passed as the first parameter
			if (lua_gettop(L) != totalArgCount) {
				std::cout << "[LUA API]: calling function " << std::hex << functionLocation << " with too few arguments;" << std::endl;
				return luaL_error(L, ("[LUA API]: calling function " + std::to_string(functionLocation) + " with too few arguments;").c_str());
			}

			for (int i = 0; i < luaHookedFunctionArgCount; i++) {
				fakeStack[i] = lua_tointeger(L, i + 1 + 1); // i+1+1 (1 to offset 0-base and 1 because this-parameter is ignored
			}

			currentECXValue = lua_tointeger(L, 1); // this parameter
		}
		else {
			if (lua_gettop(L) != luaHookedFunctionArgCount) { // + 0
				std::cout << "[LUA API]: calling function " << std::hex << functionLocation << " with too few arguments;" << std::endl;
				return luaL_error(L, ("[LUA API]: calling function " + std::to_string(functionLocation) + " with too few arguments;").c_str());
			}

			for (int i = 0; i < luaHookedFunctionArgCount; i++) {
				fakeStack[i] = lua_tointeger(L, i + 1); // i + 1 to offset the 0-base
			}


		}

		__asm {
			mov ecx, luaHookedFunctionArgCount;
		loopbegin:
			cmp ecx, 0;
			jle done;
			dec ecx;
			mov eax, fakeStack[ecx * 4];
			push eax;
			jmp loopbegin;
		done:
			mov ecx, luaCallingConvention;// FAIL! this parameter is not valid anymore at this point!
			cmp ecx, 0;
			je caller;
			jmp callee;
		caller:
			mov ecx, luaHookedFunctionArgCount;
			mov eax, newOriginalFunctionLocation;
			cmp ecx, 0;
			je add0x00;
			cmp ecx, 1;
			je add0x04;
			cmp ecx, 2;
			je add0x08;
			cmp ecx, 3;
			je add0x0C;
			cmp ecx, 4;
			je add0x10;
			cmp ecx, 5;
			je add0x14;
			cmp ecx, 6;
			je add0x18;
			cmp ecx, 7;
			je add0x1C;
			cmp ecx, 8;
			je add0x20;
		add0x00:
			call eax;
			add esp, 0x00;
			jmp eor;
		add0x04:
			call eax;
			add esp, 0x04;
			jmp eor;
		add0x08:
			call eax;
			add esp, 0x08;
			jmp eor;
		add0x0C:
			call eax;
			add esp, 0x0C;
			jmp eor;
		add0x10:
			call eax;
			add esp, 0x10;
			jmp eor;
		add0x14:
			call eax;
			add esp, 0x14;
			jmp eor;
		add0x18:
			call eax;
			add esp, 0x18;
			jmp eor;
		add0x1c:
			call eax;
			add esp, 0x1c;
			jmp eor;
		add0x20:
			call eax;
			add esp, 0x20;
			jmp eor;
		callee:
			mov eax, newOriginalFunctionLocation;
			mov ecx, currentECXValue;
			call eax;
		eor:
		}

		DWORD result;
		__asm {
			mov result, eax;
		}
		lua_pushinteger(L, result);

		return 1;
	}

	void __declspec(naked) LuaLandingFromCpp() {
		__asm {
			// pop the previous EIP+5 into eax
			pop eax;
			push ecx;
			sub eax, 5;
			push eax;
			// set up the right global variables for the current hook
			call SetLuaHookedFunctionParameters;

			//At this point, we should have the original call stack
			mov eax, esp;
			// compensate for the return address we have on the stack...
			add eax, 4;
			push eax;

			mov eax, [luaCallingConvention];
			cmp eax, 0;
			je retNone;
			cmp eax, 1; // this cmp is bullshit for now, because calling convention 0 is the only with caller cleanup.
			mov eax, [luaHookedFunctionArgCount];
			cmp eax, 0;
			je ret0x0;
			cmp eax, 1;
			je ret0x4;
			cmp eax, 2;
			je ret0x8;
			cmp eax, 3;
			je ret0xC;
			cmp eax, 4;
			je ret0x10;
			cmp eax, 5;
			je ret0x14;
			cmp eax, 6;
			je ret0x18;
			cmp eax, 7;
			je ret0x1C;
			cmp eax, 8;
			je ret0x20;
			jmp retNone;

		ret0x0:
			call executeLuaHook;
			ret 0x0;

		ret0x4:
			call executeLuaHook;
			ret 0x4;

		ret0x8:
			call executeLuaHook;
			ret 0x8;

		ret0xC:
			call executeLuaHook;
			ret 0xC;

		ret0x10:
			call executeLuaHook;
			ret 0x10;

		ret0x14:
			call executeLuaHook;
			ret 0x14;

		ret0x18:
			call executeLuaHook;
			ret 0x18;

		ret0x1C:
			call executeLuaHook;
			ret 0x1C;

		ret0x20:
			call executeLuaHook;
			ret 0x20;

		retNone:
			call executeLuaHook;
			ret;
		}
	}

	class LuaDetour {
	public:
		std::string luaDetourFunctionName;
		DWORD detourReturnLocation;
		int luaTableRef;

		LuaDetour(std::string luaDetourFunctionName, DWORD detourReturnLocation) {
			this->luaDetourFunctionName = luaDetourFunctionName;
			this->detourReturnLocation = detourReturnLocation;
			this->luaTableRef = -1;
		}
	};

	std::map<DWORD, std::shared_ptr<LuaDetour>> detourTargetMap;
	DWORD currentDetourSource;
	DWORD currentDetourReturn;
	std::string currentDetourTarget;

	// lua calls this as: detourCode(hookedFunctionName, address, hookSize)
	// or as: detourCode(hookedFunctionName, address, hookSize, env)
	int luaDetourCode(lua_State* L) {
		if (lua_gettop(L) < 3 || lua_gettop(L) > 4) {
			return luaL_error(L, "expecting exactly 3 or 4 arguments");
		}

		std::string luaOriginal = lua_tostring(L, 1);
		DWORD address = lua_tointeger(L, 2);
		int hookSize = lua_tointeger(L, 3);

		DWORD ret;
		DoCreateCallHook(address, (DWORD)detourLandingFunction, hookSize, ret);

		detourTargetMap[address] = std::make_shared<LuaDetour>(luaOriginal, ret);

		if (lua_gettop(L) == 4) {
			// push the env on top of the stack again.
			lua_pushvalue(L, 4); 
			detourTargetMap[address]->luaTableRef = luaL_ref(L, LUA_REGISTRYINDEX);
		}
		else {
			// assume the global namespace.
		}

		return 0;
	}

	void __stdcall GetDetourLuaTargetAndCallTheLuaFunction(DWORD address, DWORD* registers) {
		bool exists = detourTargetMap.count(address) == 1;
		if (!exists) {
			assert(exists);
		}
		std::shared_ptr<LuaDetour> entry = detourTargetMap[address];
		currentDetourReturn = entry->detourReturnLocation;
		int retLoc = entry->detourReturnLocation;

		const std::vector<std::string> order = { "EDI", "ESI", "EBP", "ESP", "EBX", "EDX", "ECX", "EAX" };

		if (entry->luaTableRef == -1) {
			lua_getglobal(L, entry->luaDetourFunctionName.c_str());
		}
		else {
			lua_rawgeti(L, LUA_REGISTRYINDEX, entry->luaTableRef);
			lua_getfield(L, -1, entry->luaDetourFunctionName.c_str());
			lua_remove(L, lua_absindex(L, -2)); // remove the table
		}
		
		if (lua_isfunction(L, -1)) {
			lua_createtable(L, 0, 8);

			for (int i = 0; i < order.size(); i++) {
				lua_pushstring(L, order[i].c_str());
				lua_pushinteger(L, registers[i]);
				lua_settable(L, -3);  /* 3rd element from the stack top */
			}

			// We call the function and pass 1 argument and expect 1 argument in return.
			if (lua_pcall(L, 1, 1, 0) == LUA_OK) {
				luaErrorLevel = 0;
				luaErrorMsg = "";
				if (lua_istable(L, -1)) {
					/* table is in the stack at index 't' */
					lua_pushnil(L);  /* first key */
					while (lua_next(L, -2) != 0) {
						/* uses 'key' (at index -2) and 'value' (at index -1) */
						if (!lua_isstring(L, -2)) {
							lua_pushliteral(L, "The return value table must have string keys");
							lua_error(L);
							currentDetourReturn = retLoc;
							return;
						}
						if (!lua_isinteger(L, -1)) {
							lua_pushliteral(L, "The return value table must have integer values");
							lua_error(L);
							currentDetourReturn = retLoc;
							return;
						}

						std::string key = lua_tostring(L, -2);
						DWORD value = lua_tointeger(L, -1);

						std::vector<std::string>::const_iterator it = find(order.begin(), order.end(), key);
						if (it == order.end()) {
							lua_pushstring(L, ("The key does not exist: " + key).c_str());
							lua_error(L);
							currentDetourReturn = retLoc;
							return;
						}

						int index = it - order.begin();
						registers[index] = value;

						/* removes 'value'; keeps 'key' for next iteration */
						lua_pop(L, 1);
					}
					currentDetourReturn = retLoc;
					return;
				}
				else {
					luaErrorLevel = 3;
					luaErrorMsg = std::string(luaHookedFunctionName) + " is not a function";
					lua_pushstring(L, luaErrorMsg.c_str());
					lua_error(L);
				}
				lua_pop(L, 1); // pop off the return value;
				currentDetourReturn = retLoc;
				return;
			}
			else {
				luaErrorLevel = 1;
				luaErrorMsg = lua_tostring(L, -1);
				lua_pushstring(L, luaErrorMsg.c_str());
				lua_error(L);
				lua_pop(L, 1); // pop off the error message;
			}
		}
		else {
			lua_pop(L, 1); // I think we need this pop, because the getglobal does a push that would otherwise be popped by pcall.
			luaErrorLevel = 2;
			luaErrorMsg = std::string(luaHookedFunctionName) + " is not a function";
			lua_pushstring(L, luaErrorMsg.c_str());
			lua_error(L);
		}

		std::cout << "[LUA API]: " << std::string(luaErrorMsg) << std::endl;
		currentDetourReturn = retLoc;
	}

	void __declspec(naked) detourLandingFunction() {
		__asm {

			pushfd; // pushes 1 element
			pushad; // pushes 8 elements

			mov ecx, esp; // store a pointer to the register values on the stack.

			mov eax, [esp + (9 * 0x04)]; // the 9th element will be the return address from the detour.
			sub eax, 5; // subtract 5 because a jump is 5 long to get the origin address.
			push ecx; // push the register array;
			push eax; // set this as an argument to the function.

			// set up the right global variables for the current detour
			call GetDetourLuaTargetAndCallTheLuaFunction; // this function also should set currentDetourReturn;

			popad;
			popfd;

			add esp, 4; // remove the address that was pushed by the call detour

			jmp currentDetourReturn;
		}
	}

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
		{"allocateCode", luaAllocateRWE},

		{"readByte", luaReadByte},
		{"readSmallInteger", luaReadSmallInteger},
		{"readInteger", luaReadInteger},
		{"readString", luaReadString},
		{"readBytes", luaReadBytes},

		{"writeByte", luaWriteByte},
		{"writeSmallInteger", luaWriteSmallInteger},
		{"writeInteger", luaWriteInteger},
		{"writeBytes", luaWriteBytes},
		{"writeCode", luaWriteCode},

		{"copyMemory", luaMemCpy},

		{"registerString", registerString},

		{"scanForAOB", luaScanForAOB},
		{NULL, NULL} /* end of array */
	};


	void initializePrintRedirect(lua_State* L) {
		lua_pushglobaltable(L);
		luaL_setfuncs(L, printlib, 0);
		lua_pop(L, 1);
	}

	/**
	 * This function registers the available functions in a table with a certain name if desired.
	 * When a custom name is specified, a table with that name will be put in the global namespace. If the namespace is set to null, the table is left on the lua stack.
	 * 
	 * \param L the lua state
	 * \param apiNamespace the namespace to put the functions in, can be "global", NULL, or a custom name. 
	 */
	void initializeLuaAPI(lua_State* L, std::string apiNamespace) {
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


	void setupPackagePath(lua_State *L, std::string packagePath) {
		lua_getglobal(L, "package");
		lua_pushstring(L, "path");
		lua_pushstring(L, packagePath.c_str());
		lua_settable(L, -3);
		lua_pop(L, 1);
	}

	void runBootstrapFile(lua_State* L, std::string bootstrapFilePath) {
		int stackSize = lua_gettop(L);

		int r = luaL_dofile(L, bootstrapFilePath.c_str());

		if (r == LUA_OK) {
			std::cout << "[LUA]: loaded LUA API." << std::endl;

			lua_pop(L, lua_gettop(L) - stackSize);
		}
		else {
			std::string errormsg = lua_tostring(L, -1);
			std::cout << "[LUA]: failed to load LUA API: " << errormsg << std::endl;
			lua_pop(L, 1); // pop off the error message;
		}
	}

	void initializeLua() {
		L = luaL_newstate();
	}

	bool initializeCodeHeap() {
		if (codeHeap == 0) {
			codeHeap = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, 0, 0); // start out with one page
		}
		return codeHeap != 0;
	}

	void initialize(std::string bootstrapFilePath, std::string packagePath, bool initializePrintRedirect) {
		initializeLua();

		initializeCodeHeap();

		luaL_openlibs(L);

		setupPackagePath(L, packagePath);

		if(initializePrintRedirect) LuaAPI::initializePrintRedirect(L);

		initializeLuaAPI(L, "global");
		

		runBootstrapFile(L, bootstrapFilePath);
	}

	void deinitialize() {
		lua_close(L);

		if (codeHeap != 0) {
			HeapDestroy(codeHeap);
		}
	}

	void executeSnippet(std::string code) {
		int before = lua_gettop(L);
		int r = luaL_dostring(L, code.c_str());
		if (r == LUA_OK) {
			int after = lua_gettop(L);
			for (int i = before; i < after; i++) {
				int index = before - (i + 1);
				if (lua_isstring(L, index)) {
					std::cout << lua_tostring(L, index) << std::endl;
				}
				else if (lua_isnumber(L, index)) {
					std::cout << lua_tonumber(L, index) << std::endl;
				}
				else if (lua_isboolean(L, index)) {
					std::cout << lua_toboolean(L, index) << std::endl;
				}
				else if (lua_isnil(L, index)) {
					std::cout << "nil" << std::endl;
				}
				else {
					std::cout << "[object]" << std::endl;
				}
			}
			lua_pop(L, after - before);

		}
		else {
			std::string errormsg = lua_tostring(L, -1);
			std::cout << "[LUA]: " << errormsg << std::endl;
			lua_pop(L, 1); // pop off the error message;
		}
	}

	int getCurrentStackSize() {
		return lua_gettop(L);
	}

	lua_State* getLuaState() {
		return L;
	}


}