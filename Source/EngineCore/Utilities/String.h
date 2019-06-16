#pragma once

// =====================================================================
// String.h : Defines some useful string utility functions
// =====================================================================

#include <string>
#include "Common/CommonStd.h"

// Does a classic * & ? pattern match on a file name - this is case sensitive!
extern BOOL WildcardMatch(const char* pat, const char* str);


extern std::string ws2s(const std::wstring& s);

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