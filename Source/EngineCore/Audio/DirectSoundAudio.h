#pragma once

//========================================================================
// DirectSoundAudio.h : Implements the audio interfaces for DirectSound 
//========================================================================

#include "../Common/CommonStd.h"
#include "Audio.h"

// DirectSound includes
#include <dsound.h>
#include <mmsystem.h>

// ---------------------------------------------------------------------------
// class DirectSoundAudioBuffer					-Chapter 13, page 420
//
// Implements the rest of the IAudioBuffer interface left out by AudioBuffer.
// If you're interested in implementing a sound system using OpenAL you'd create
// a class OpenALAudioBuffer from AudioBuffer.
//
// ---------------------------------------------------------------------------

class DirectSoundAudioBuffer : public AudioBuffer
{
protected:
	LPDIRECTSOUNDBUFFER m_Sample;

public:
	DirectSoundAudioBuffer(LPDIRECTSOUNDBUFFER sample, std::shared_ptr<ResHandle> resource);
	virtual void* VGet();
	virtual bool VOnRestore();

	virtual bool VPlay(int volume, bool looping);
	virtual bool VPause();
	virtual bool VStop();
	virtual bool VResume();

	virtual bool VTogglePause();
	virtual bool VIsPlaying();
	virtual void VSetVolume(int volume);
	virtual void VSetPosition(unsigned long newPosition);

	virtual float VGetProgress();

private:
	HRESULT FillBufferWithSound();
	HRESULT RestoreBuffer(BOOL* pbWasRestored);
};


// ---------------------------------------------------------------------------
// class DirectSoundAudio							-Chapter 13, page 414
//
// Implements the rest of the IAudio interface left out by Audio.
// If you are interested in implementing a sound system using OpenAL
// you'd create a class OpenALAudioBuffer from AudioBuffer.
//
// ---------------------------------------------------------------------------

class DirectSoundAudio : public Audio
{
public:
	DirectSoundAudio() { m_pDS = nullptr; }
	virtual bool VActive() { return m_pDS != nullptr; }

	virtual IAudioBuffer* VInitAudioBuffer(std::shared_ptr<ResHandle> handle);
	virtual void VReleaseAudioBuffer(IAudioBuffer* audioBuffer);

	virtual void VShutdown();
	virtual bool VInitialize(HWND hWnd);

protected:
	IDirectSound8* m_pDS;

	HRESULT SetPrimaryBufferFormat(DWORD dwPrimaryChannels,
								   DWORD dwPrimaryFreq,
								   DWORD dwPrimaryBitRate);
};