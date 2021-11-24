
#include "framework.h"
#include <vector>
#include <regex>
#include <string>
#include <iostream>

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
		for (DWORD i = 0; i < (dwLen - strlen((char*)szMask)); i++)
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

	DWORD Scan(char* content, char* mask, DWORD min, DWORD max)
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		_MEMORY_BASIC_INFORMATION32 mbi;
		DWORD address = min;
		int remainder = 0;
		
		while (VirtualQuery((LPCVOID) address, ((MEMORY_BASIC_INFORMATION*)&mbi), sizeof(MEMORY_BASIC_INFORMATION)) != 0) {
			if (mbi.State == MEM_COMMIT) {
				if ((mbi.Type != MEM_MAPPED) && (mbi.Type != MEM_PRIVATE)) {
					if ((mbi.Protect & PAGE_NOACCESS) == 0) {
						// address = 0x401002
						// mbi.BaseAddress = 0x401000
						DWORD needle = FindPattern(address, mbi.RegionSize, (BYTE*)content, mask);
						if (needle == 0) {
							// address = 0x401000
							address = mbi.BaseAddress;
						}
						else {
							return needle;
						}
					}
				}
			}
			
			// address = 0x59E000
			address += mbi.RegionSize;
			if (address > max) {
				return 0;
			}
		}
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

		return Scan((char*)(&AOB::HexToBytes(content)[0]), (char*)mask.c_str(), min, max);
	}

	// TODO: find all?
	// example: Find("57 E8 7B C0 10 ?")
	DWORD Find(std::string ucp_aob_spec)
	{
		return FindInRange(ucp_aob_spec, 0x400000, 0x7FFFFFFF);
	}
}
