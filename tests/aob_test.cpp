#include "../framework.h"
#include <string>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include "../AOB.h"

#pragma section(".aobdata", read, write)
#pragma section(".aobexec", read, execute)
__declspec(allocate(".aobdata")) __declspec(align(4096)) unsigned char data[16384];
__declspec(allocate(".aobexec")) __declspec(align(4096)) unsigned char code[16384];

static DWORD address(const void* p) { return DWORD(reinterpret_cast<uintptr_t>(p)); }
static void check(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}
static void protect(void* p, DWORD flags) {
    DWORD old;
    check(VirtualProtect(p, 4096, flags, &old) != 0, "VirtualProtect");
}

int main() {
    try {
        const char* text = "UniqueAobFixture!";
        const std::string pattern = "55 6E 69 71 75 65 41 6F 62 46 69 78 74 75 72 65 21";
        const size_t size = strlen(text);
        memset(data, 0, sizeof(data));
        memcpy(data + 100, text, size);
        const DWORD begin = address(data);
        check(AOB::FindInRange(pattern, begin + 100, begin + 100 + DWORD(size) - 1) == begin + 100, "exact-size range");
        check(AOB::FindInRange(pattern, begin, begin + 100 + DWORD(size) - 1) == begin + 100, "last legal start");
        check(!AOB::FindInRange(pattern, begin, begin + 100 + DWORD(size) - 2), "upper bound clips pattern");
        check(!AOB::FindInRange(pattern, begin + 101, begin + 200), "lower bound clips pattern");
        check(!AOB::FindInRange(pattern, begin + 200, begin + 199), "inverted range");
        check(!AOB::FindInRange(pattern, begin + 100, begin + 100), "pattern longer than range");
        check(AOB::FindInRange("55 ? 69 ? 75 65", begin + 100, begin + 105) == begin + 100, "wildcards");

        // A last-byte match next to an inaccessible page must not over-read it.
        data[4095] = 0xDA;
        protect(data + 4096, PAGE_NOACCESS);
        check(AOB::FindInRange("DA", begin + 4095, begin + 8191) == begin + 4095, "last byte before noaccess");
        check(!AOB::FindInRange(pattern, begin + 4095, begin + 8191), "interior region start before noaccess");
        protect(data + 4096, PAGE_READWRITE | PAGE_GUARD);
        check(!AOB::FindInRange(pattern, begin + 4095, begin + 8191), "guard page skipped");
        MEMORY_BASIC_INFORMATION info = {};
        VirtualQuery(data + 4096, &info, sizeof(info));
        check((info.Protect & PAGE_GUARD) != 0, "guard not consumed");
        protect(data + 4096, PAGE_READWRITE);

        // Protection changes do not make an otherwise readable match disappear.
        memcpy(data + 4096 - 8, text, size);
        protect(data, PAGE_READONLY);
        check(AOB::FindInRange(pattern, begin + 4000, begin + 4200) == begin + 4096 - 8, "cross-region match");
        protect(data, PAGE_READWRITE);

        #ifndef AOB_TEST_LEGACY
        DWORD second = 0;
        check(!AOB::FindInMainModule(pattern, second) && !second, "data and literals excluded");
        memcpy(code + 4096 - 8, text, size);
        protect(code, PAGE_EXECUTE_READ);
        check(AOB::FindInMainModule(pattern, second) == address(code + 4096 - 8) && !second, "unique executable cross-region match");
        memcpy(code + 8200, text, size);
        check(AOB::FindInMainModule(pattern, second) == address(code + 4096 - 8) && second == address(code + 8200), "two executable matches");
        protect(code + 4096, PAGE_READWRITE);
        check(AOB::FindInMainModule(pattern, second) == address(code + 8200) && !second, "non-executable gap not searched");
        protect(code + 4096, PAGE_EXECUTE_READWRITE);
        protect(code, PAGE_EXECUTE_READWRITE);
        memset(code, 0, sizeof(code));
        memset(code + 100, 0xDA, 18);
        const std::string overlapping = "DA DA DA DA DA DA DA DA DA DA DA DA DA DA DA DA DA";
        check(AOB::FindInMainModule(overlapping, second) == address(code + 100) && second == address(code + 101), "overlapping duplicates");
        protect(code, PAGE_EXECUTE_READWRITE | PAGE_GUARD);
        bool rejected = false;
        try { AOB::FindInMainModule(pattern, second); } catch (const std::runtime_error&) { rejected = true; }
        protect(code, PAGE_EXECUTE_READWRITE);
        check(rejected, "guarded executable code reports failure");
        #endif
        std::cout << "AOB native boundary and ambiguity checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
