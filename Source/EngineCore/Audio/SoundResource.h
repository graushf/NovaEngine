#pragma once

//========================================================================
// SoundResource.h : 
//========================================================================

#include <mmsystem.h>

#include "Common/CommonStd.h"
#include "ResourceCache/ResCache.h"

//
// class SoundResourceExtraData				- Chapter 13, page 399
//
class SoundResourceExtraData : public IResourceExtraData
{
	friend class WaveResourceLoader;
	friend class OggResourceLoader;

public:
	SoundResourceExtraData();
	virtual ~SoundResourceExtraData() { }
	virtual std::string VToString() { return "SoundResourceExtraData"; }
	enum SoundType GetSoundType() { return m_SoundType; }
	WAVEFORMATEX const* GerFormat() { return &m_WavFormatEx; }
	int GetLengthMilli() const { return m_LengthMilli; }

protected:		
	enum SoundType m_SoundType;				// is this an Ogg, WAV, etc. ?
	bool m_bInitialized;					// has the sound been initialized
	WAVEFORMATEX m_WavFormatEx;				// description of the PCM format
	int m_LengthMilli;						// how long the sound is in milliseconds
};

//
// class WaveResourceLoader						- Chapter 13, page 399
//
class WaveResourceLoader : public IResourceLoader
{
public:
	virtual bool VUseRawFile() { return false; }
	virtual bool VDiscardRawBufferAfterLoad() { return true; }
	virtual unsigned int VGetLoadedResourceSize(char* rawBuffer, unsigned int rawSize);
	virtual bool VLoadResource(char* rawBuffer, unsigned int rawSize, std::shared_ptr<ResHandle> handle);
	virtual std::string VGetPattern() { return "*.wav"; }

protected:
	bool ParseWave(char* wavStream, size_t length, std::shared_ptr<ResHandle> handle);
};

//
// class OggResourceLoader						- Chapter 13, page 399
//
class OggResourceLoader : public IResourceLoader
{
public:
	virtual bool VUseRawFile() { return false; }
	virtual bool VDiscardRawBufferAfterLoad() { return true; }
	virtual unsigned int VGetLoadedResourceSize(char* rawBuffer, unsigned int rawSize);
	virtual bool VLoadResource(char* rawBuffer, unsigned int rawSize, std::shared_ptr<ResHandle> handle);
	virtual std::string VGetPattern() { return "*.ogg"; }

protected:
	bool ParseOgg(char* oggStream, size_t length, std::shared_ptr<ResHandle> handle);
};