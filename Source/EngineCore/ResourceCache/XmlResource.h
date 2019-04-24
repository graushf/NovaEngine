#pragma once

#include "Common/CommonStd.h"
#include <tinyxml.h>

class XmlResourceLoader : public IResourceLoader
{
public:
	

	// convenience function
	static TiXmlElement* LoadAndReturnRootXmlElement(const char* resourceString);
};