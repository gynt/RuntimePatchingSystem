#pragma once

namespace AOB {
	int Scan(char* content, char* mask, DWORD min, DWORD max);
	std::vector<char> HexToBytes(const std::string& hex);

	//Consider: https://github.com/CvX/hadesmem
	DWORD FindPattern(DWORD dwAddress, DWORD dwLen, BYTE* bMask, char* szMask);
	DWORD Find(std::string ucp_aob);
	DWORD FindInRange(std::string ucp_aob_spec, DWORD min, DWORD max);
}