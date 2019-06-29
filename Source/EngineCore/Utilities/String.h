#pragma once

// =====================================================================
// String.h : Defines some useful string utility functions
// =====================================================================

#include <string>
#include "Common/CommonStd.h"

#define MAX_DIGITS_IN_INT 12		// max number of digits in an int (-2147483647 = 11 digits, +1 for the '\0')
typedef std::vector<std::string> StringVec;

// Does a classic * & ? pattern match on a file name - this is case sensitive!
extern BOOL WildcardMatch(const char* pat, const char* str);


extern HRESULT AnsiToWideCch(WCHAR* dest, const CHAR* src, int charCount);


extern std::string ToStr(int num, int base = 10);


extern std::string ws2s(const std::wstring& s);


// Splits a string by the delimeter into a vector of strings. For example, say you have the following strings:
// std::string test("one, two, three");
// You could call Split() like this:
// Split(test, outVec, ',');
// outVec will have the following values:
// "one", "two", "three"
void Split(const std::string& str, StringVec& vec, char delimiter);


// A hashed string. It retains the initial (ANSI) string in addition to the hash value for easy reference.
class HashedString
{
public:
	explicit HashedString(char const * const pIdentString)
		: m_ident(hash_name(pIdentString)),
		m_identStr(pIdentString)
	{
	}

	unsigned long getHashValue(void) const
	{
		return reinterpret_cast<unsigned long>(m_ident);
	}

	const std::string & getStr() const
	{
		return m_identStr;
	}

	static void* hash_name(char const* pIdentStr);

	bool operator< (HashedString const & o) const
	{
		bool r = (getHashValue() < o.getHashValue());
		return r;
	}

	bool operator== (HashedString const & o) const
	{
		bool r = (getHashValue() == o.getHashValue());
		return r;
	}

private:

	// note: m_ident is stored as void* not as int, so that in
	// the debugget it will show up as hex-values instead of
	// integer values. This is a bit more representative of what
	// we're doing here and makes it easy to allow external code
	// to assign event types as desired.
	
	void *				m_ident;
	std::string			m_identStr;
};