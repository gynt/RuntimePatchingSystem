#pragma once

#include "framework.h"

class ProcessMemory
{
public:
    static ProcessMemory& getInstance()
    {
        static ProcessMemory    instance; // Guaranteed to be destroyed.
                              // Instantiated on first use.

        instance.codeHeap = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, 0, 0);
        instance.dataHeap = GetProcessHeap();

        return instance;
    }
private:
    ProcessMemory() {}                    // Constructor? (the {} brackets) are needed here.

    // C++ 11
    // =======
    // We can use the better technique of deleting the methods
    // we don't want.
public:
    ProcessMemory(ProcessMemory const&) = delete;
    void operator=(ProcessMemory const&) = delete;

    HANDLE codeHeap = 0;
    HANDLE dataHeap = 0;

    // Note: Scott Meyers mentions in his Effective Modern
    //       C++ book, that deleted functions should generally
    //       be public as it results in better error messages
    //       due to the compilers behavior to check accessibility
    //       before deleted status
};