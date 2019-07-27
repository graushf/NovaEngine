#pragma once

// ================================================================
// ScriptResource.h 
// ================================================================

// --------------------------------------------------------------------
// File:			ScriptResource.h
//
// Purpose:		The declaration of a quick'n diry ZIP file reader class.
//				Original code from Javier Arevalo.
//				Get zlib from http://www.cdrom.com/pub/infozip/zlib/
//
// class XmlFile - Chapter 7, page 186
// --------------------------------------------------------------------

#include "../Common/CommonStd.h"
#include "../ResourceCache/ResCache.h"

class ScriptResourceLoader : public IResourceLoader
{
public:
	virtual bool VUseRawFile() { return false; }
	virtual bool VDiscardRawBufferAfterLoad() { return true; }
	virtual bool VAddNullZero() { return true; }
	virtual unsigned int VGetLoadedResourceSize(char* rawBuffer, unsigned int rawSize) { return rawSize; }
	virtual bool VLoadResource(char* rawBuffer, unsigned int rawSize, std::shared_ptr<ResHandle> handle);
	virtual std::string VGetPattern() { return "*.lua"; }
};