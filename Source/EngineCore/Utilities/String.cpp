#include "String.h"
#include "Common/CommonStd.h"

// The following function was found on http://xoomer.virgilio.it/acantato/dev/wildcard/wildmatch.html, where it was attributed to 
// the C/C++ Users Journal, written by Mike Cornelison. It is a little ugly, but it is FAST. Use this as an excercise in not reinventing the
// wheel, even if you see gotos. 

BOOL WildcardMatch(const char* pat, const char* str)
{
	int i, star;

new_segment:

	star = 0;
	if (*pat == '*') {
		star = 1;
		do { pat++; } while (*pat == '*'); /* enddo */
	} /* endif */

test_match:

	for (i = 0; pat[i] && (pat[i] != '*'); i++) {
		//if (mapCaseTable[str[i]] != mapCaseTable[pat[i]]) {
		if (str[i] != pat[i]) {
			if (!str[i]) return 0;
			if ((pat[i] == '?') && (str[i] != '.')) continue;
			if (!star) return 0;
			str++;
			goto test_match;
		}
	}
	if (pat[i] == '*') {
		str += i;
		pat += i;
		goto new_segment;
	}
	if (!str[i]) return 1;
	if (i && pat[i - 1] == '*') return 1;
	if (!star) return 0;
	str++;
	goto test_match;
}

std::string ws2s(const std::wstring& s)
{
	int slength = (int)s.length() + 1;
	int len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0) - 1;
	std::string r(len, '\0');
	WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0);
	return r;
}


HRESULT AnsiToWideCch(WCHAR* wstrDestination, const CHAR* strSource, int cchDestChar)
{
	if (wstrDestination == nullptr || strSource == nullptr || cchDestChar < 1)
		return E_INVALIDARG;

	int nResult = MultiByteToWideChar(CP_ACP, 0, strSource, -1,
		wstrDestination, cchDestChar);

	wstrDestination[cchDestChar - 1] = 0;

	if (nResult == 0)
		return E_FAIL;

	return S_OK;
}