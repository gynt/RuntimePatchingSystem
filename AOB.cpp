
#include "framework.h"
#include <vector>
#include <regex>
#include <string>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <stdexcept>

#include "AOB.h"


namespace AOB {

	bool bCompare(const BYTE* pData, const BYTE* bMask, const char* szMask)
	{
		for (; *szMask; ++szMask, ++pData, ++bMask)
			if (*szMask == 'x' && *pData != *bMask)
				return 0;
		return (*szMask) == NULL;
	}
	DWORD FindPattern(DWORD dwAddress, DWORD dwLen, BYTE* bMask, char* szMask)
	{
		const size_t length = strlen(szMask);
		if (length == 0 || length > dwLen) return 0;
		for (DWORD i = 0; i <= dwLen - length; i++)
		{
			if (bCompare((BYTE*)(dwAddress + i), bMask, szMask))
			{
				return (DWORD)(dwAddress + i);
			}
		}
		return 0;
	}

	std::vector<char> HexToBytes(const std::string& hex)
	{
		std::vector<char> bytes;

		for (unsigned int i = 0; i < hex.length(); i += 2) {
			std::string byteString = hex.substr(i, 2);
			char byte = (char)strtol(byteString.c_str(), NULL, 16);
			bytes.push_back(byte);
		}

		return bytes;
	}

	static bool ReadableImage(const MEMORY_BASIC_INFORMATION& region)
	{
		return region.State == MEM_COMMIT && region.Type == MEM_IMAGE &&
			(region.Protect & (PAGE_NOACCESS | PAGE_GUARD)) == 0;
	}

	// Both bounds are inclusive. Merge adjacent readable regions so a pattern
	// spanning a protection boundary is considered, without reading guarded pages.
	DWORD Scan(char* content, char* mask, DWORD min, DWORD max)
	{
		const uint64_t stop = uint64_t(max) + 1;
		uint64_t address = min, runStart = min;
		while (address < stop) {
			MEMORY_BASIC_INFORMATION mbi = {};
			const bool queried = VirtualQuery(reinterpret_cast<LPCVOID>(uintptr_t(address)), &mbi, sizeof(mbi)) != 0;
			const uint64_t end = queried ? uint64_t(reinterpret_cast<uintptr_t>(mbi.BaseAddress)) + mbi.RegionSize : address;
			if (!queried || end <= address || !ReadableImage(mbi)) {
				if (address > runStart) {
					const DWORD found = FindPattern(DWORD(runStart), DWORD(address - runStart), reinterpret_cast<BYTE*>(content), mask);
					if (found) return found;
				}
				if (!queried || end <= address) return 0;
				runStart = (std::min)(end, stop);
			}
			address = (std::min)(end, stop);
		}
		if (address > runStart)
			return FindPattern(DWORD(runStart), DWORD(address - runStart), reinterpret_cast<BYTE*>(content), mask);
		return 0;
	}


	DWORD FindInRange(std::string ucp_aob_spec, DWORD min, DWORD max) {
		std::string haystack = ucp_aob_spec;
		std::regex target("([A-Fa-f0-9]{2})|([?]+)");
		std::smatch sm;

		std::string content("");
		std::string mask("");

		while (std::regex_search(haystack, sm, target))
		{
			if (sm[0] == "?") {
				mask += " ";
				content += "FF"; //Or 00? We just need dummy content here.
			}
			else {
				mask += "x";
				content += sm[0].str();
			}

			haystack = sm.suffix();
		}

		auto bytes = HexToBytes(content);
		if (bytes.empty()) return 0;
		return Scan(bytes.data(), const_cast<char*>(mask.c_str()), min, max);
	}

	DWORD FindInMainModule(std::string pattern, DWORD& second)
	{
		second = 0;
		const HMODULE module = GetModuleHandleW(nullptr);
		if (!module) throw std::runtime_error("Cannot locate the main executable");
		uint64_t address = reinterpret_cast<uintptr_t>(module);
		std::vector<std::pair<uint64_t, uint64_t>> ranges;
		while (address <= MAXDWORD) {
			MEMORY_BASIC_INFORMATION mbi = {};
			if (!VirtualQuery(reinterpret_cast<LPCVOID>(uintptr_t(address)), &mbi, sizeof(mbi)))
				throw std::runtime_error("Cannot query the main executable's memory");
			if (mbi.AllocationBase != module) break;
			const uint64_t end = uint64_t(reinterpret_cast<uintptr_t>(mbi.BaseAddress)) + mbi.RegionSize;
			if (end <= address || end > uint64_t(MAXDWORD) + 1)
				throw std::runtime_error("Invalid main executable memory range");
			const DWORD executable = PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
			if (mbi.Protect & executable) {
				if (!ReadableImage(mbi)) throw std::runtime_error("Main executable code is not accessible");
				if (!ranges.empty() && ranges.back().second == address) ranges.back().second = end;
				else ranges.emplace_back(address, end);
			}
			address = end;
		}
		DWORD first = 0;
		for (const auto& range : ranges) {
			const DWORD found = FindInRange(pattern, DWORD(range.first), DWORD(range.second - 1));
			if (found) {
				if (first) { second = found; return first; }
				first = found;
				if (uint64_t(found) + 1 < range.second)
					second = FindInRange(pattern, found + 1, DWORD(range.second - 1));
				if (second) return first;
			}
		}
		return first;
	}

	// TODO: find all?
	// example: Find("57 E8 7B C0 10 ?")
	DWORD Find(std::string ucp_aob_spec)
	{
		return FindInRange(ucp_aob_spec, 0x400000, 0x7FFFFFFF);
	}
}
