
#include "framework.h"
#include <vector>
#include <regex>
#include <string>
#include <iostream>

#include "AOB.h"


namespace AOB {

	HANDLE hProcess = 0;
	DWORD pid = 0;

	BYTE* ReadMemoryAddress(DWORD address)
	{
		static char buffer[1024];
		memcpy(buffer, (void*)address, sizeof(buffer));
		return reinterpret_cast<BYTE*> (buffer);
	}

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
		MEMORY_BASIC_INFORMATION mi;
		for (DWORD lpAddr = min; lpAddr < max; lpAddr += si.dwPageSize)
		{
			//if (lpAddr > 0x59dfff) {
			//	//Anything beyond this address is not of interest?
			//	return 0;
			//}
			SIZE_T vq = VirtualQuery((void*)lpAddr, &mi, si.dwPageSize);
			if (vq == ERROR_INVALID_PARAMETER || vq == 0) break;
			if (mi.State != MEM_COMMIT) continue;
			if ((mi.Type == MEM_MAPPED) || (mi.Type == MEM_PRIVATE)) continue;
			if ((mi.Protect & PAGE_NOACCESS)) continue;
			DWORD addr = FindPattern(lpAddr, si.dwPageSize, (BYTE*)content, mask);
			if (addr != 0)
			{
				return addr;
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
		return FindInRange(ucp_aob_spec, 0, 0x7FFFFFFF);
	}
}