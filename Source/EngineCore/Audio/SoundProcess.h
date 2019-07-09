#pragma once

//========================================================================
// SoundProcess.h
//========================================================================

#include "../MainLoop/Process.h"

#include "Audio.h"
#include "SoundResource.h"

// ------------------------------------------------------------
// class SoundProcess					- Chapter 13, page 426
//
//	A sound Process, not to be confused with a Sound Resource
//  (SoundResource) manages a source as it is being played. You
//	can use this class to manage timing between sounds &
//	animations.
// ------------------------------------------------------------

class SoundProcess : public Process
{
protected:
	std::shared_ptr<ResHandle> m_handle;
	std::shared_ptr<IAudioBuffer> m_AudioBuffer;

	int m_Volume;
	bool m_isLooping;

public:
	SoundProcess(std::shared_ptr<ResHandle> soundResource, int volume = 100, bool looping = false);
	virtual ~SoundProcess();

	void Play(const int volume, const bool looping);
	void Stop();

	void SetVolume(int volume);
	int GetVolume();
	int GetLengthMilli();
	bool IsSoundValid() { return m_handle != NULL; }
	bool IsPlaying();
	bool IsLooping() { return m_AudioBuffer && m_AudioBuffer->VIsLooping(); }
	float GetProgress();
	void PauseSound(void);

protected:
	virtual void VOnInit();
	virtual void VOnUpdate(unsigned long deltaMs);

	void InitializeVolume();

protected:
	SoundProcess();		// Disable Default Construction

};