
# RuntimePatchingSystem
A patching system that supports modifying, extending (detouring, hooking), and patching code in memory at runtime. Uses Lua

## Features
### Hooking functions with Lua functions

When function A has been hooked and it is called by the program, the lua function is called instead. The lua function can then optionally call the original function. The returned value of the hooked function is passed back to the program.

```
hookCode(...)
Parameters:
    luaHookCallback        - the name of the Lua function to call
    luaOriginalFunction    - what to name the original function
    hookAtAddress          - the location in memory to start the hook
    argumentCount          - the amount of arguments passed to the function (including this parameter)
    callingConvention      - specifies whether the hooked function is a (0 = cdecl, 1 = thiscall, 2 = stdcall)
    hookSize               - the size of the hook (total size of all overwritten instructions)
``` 

#### Example
```lua
-- we hook function A at address 0xABCDEF12, it is a thiscall
-- and has three arguments. We need to overwrite 2 instructions (6 bytes total)
-- we expose function A as "functionA_original" so we can call the original
hookCode("functionA_hook", "functionA_original", 0xABCDEF12, 3, 1, 6)

-- This function will be called instead of the original function A
-- This hook will make function A always result 0 
-- when its original result would have been 1
function functionA_hook(this, param_1, param_2)
    local result = functionA_original(this, param_1, param_2)
    if result == 1 then
        result = 0
    end
    return result
end
```

### Exposing functions to lua
In order to be able to call a program function, for example function B, from Lua, we have to expose it to Lua.
```
exposeCode(...)
Parameters:
    functionName           - what to name the exposed function
    address                - the location in memory to start the hook
    argmentCount           - the amount of arguments passed to the function (including this parameter)
    callingConvention      - specifies whether the hooked function is a (0 = cdecl, 1 = thiscall, 2 = stdcall)
```

#### Example
```lua
-- we expose function A (a thiscall) as "functionA" so we can call it
exposeCode("functionA", 0xABCDEF12, 3, 1)

function yourFunction()
    -- the first parameter to the function is 
    -- the "this" parameter because it is a thiscall
    local result = functionA(0x12345ABC, -1, 99)
    if result == 1 then
        result = 0
    end
    return result
end
```

### Detour code
Redirects program flow to a Lua function. Note that the memory that is overwritten to jump to Lua is executed after the Lua callback function.
The function that is called receives one parameter which is a table of all 8 x86 registers. This table should always be returned by the Lua function.
```
detourCode(...)
Parameters:
    luaCallback        - the name of the Lua function to call, receives one argument
    address            - location to put the detour at
    size               - total size of the instructions to overwrite
```

#### Example
```lua
function onDetour(registers)
    registers.EAX = 1
    return registers
end

detourCode("onDetour", 0xABCDEF, 7)
```

### Other functions
#### AOB scanning
```
scanForAOB(searchPattern[, min, max])
    searchPattern    scans the memory for this pattern
    min              address to start searching
    max              address to stop searching
    
searchPattern: hexadecimal array with question marks for wildcards.
    example: "FF A1 E? B? ?? 00"
```
#### Data functions
```
allocate(size)
    returns a pointer to a memory block of 'size' length
readBytes(address, amount)
    returns a table of bytes of size 'amount'
writeBytes(address, data)
    writes the data to the address in memory
readString(address)
    reads a 0-terminated ascii string from memory

There is also: readInteger, writeInteger, readSmallInteger, writeSmallInteger
```

#### Code functions
```
allocateCode(size)
    allocates a memory block with execution rights
writeCode(address, bytes)
    writes the (code) bytes at the address
```