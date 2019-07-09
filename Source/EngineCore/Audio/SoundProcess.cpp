//========================================================================
// SoundProcess.cpp
//========================================================================

#include "../Common/CommonStd.h"
#include "SoundProcess.h"

//
// SoundProcess::SoundProcess						- Chapter 13, page 428
//
SoundProcess::SoundProcess(std::shared_ptr<ResHandle> resource, int volume, bool looping) :
	m_handle(resource),
	m_Volume(volume),
	m_isLooping(looping)
{
	InitializeVolume();
}

//
// SoundProcess::~SoundProcess					- Chapter 13, page  428
//
SoundProcess::~SoundProcess()
{
	if (IsPlaying()) {
		Stop();
	}

	if (m_AudioBuffer) {
		g_pAudio->VReleaseAudioBuffer(m_AudioBuffer.get());
	}
}

void SoundProcess::InitializeVolume()
{
	// FUTURE WORK: Somewhere set and adjusted volume based on game options
	// m_volume = g_GraphicalApp->GetVolume(typeOfSound);
}

//
// SoundProcess::GetLengthMilli					- Chapter 13, page 430
//
int SoundProcess::GetLengthMilli()
{
	if (m_handle && m_handle->GetExtra())
	{
		std::shared_ptr<SoundResourceExtraData> extra = static_pointer_cast<SoundResourceExtraData>(m_handle->GetExtra());
		return extra->GetLengthMilli();
	}
	else {
		return 0;
	}
}

//
// SoundProcess::VOnInitialize					- Chapter 13, page 428
//
void SoundProcess::VOnInit()
{
	Process::VOnInit();

	// If the sound has never been... you know... then  Play it for the very first time.
	if (m_handle == NULL || m_handle->GetExtra() == NULL) {
		return;
	}

	// This sound will manage it's own handle in the other thread
	IAudioBuffer* buffer = g_pAudio->VInitAudioBuffer(m_handle);

	if (!buffer) {
		Fail();
		return;
	}

	m_AudioBuffer.reset(buffer);

	Play(m_Volume, m_isLooping);
}

//
// SoundProcess::OnUpdate					- Chapter 13, page 429
//
void SoundProcess::VOnUpdate(unsigned long deltaMs)
{
	if (!IsPlaying())
	{
		Succeed();
	}
}

bool SoundProcess::IsPlaying()
{
	if (!m_handle || !m_AudioBuffer) {
		return false;
	}

	return m_AudioBuffer->VIsPlaying();
}

//
// SoundProcess::SetVolume					- Chapter 13, page 430
//
void SoundProcess::SetVolume(int volume)
{
	if (m_AudioBuffer == NULL) {
		return;
	}

	//Nv_ASSERT(volume >= 0 && volume <= 100 && "Volume must be a number between 0 and 100");
	m_Volume = volume;
	m_AudioBuffer->VSetVolume(volume);
}

//
// SoundProcess::GetVolume					- Chapter 13, page 430
//
int SoundProcess::GetVolume()
{
	if (m_AudioBuffer == NULL) {
		return 0;
	}

	m_Volume = m_AudioBuffer->VGetVolume();
	return m_Volume;
}

//
// SoundProcess::PauseSound					- Chapter 13, page 430
//	NOTE: This is called TogglePause in the book
void SoundProcess::PauseSound()
{
	if (m_AudioBuffer) {
		m_AudioBuffer->VTogglePause();
	}
}

void SoundProcess::Play(const int volume, const bool looping)
{
	//Nv_ASSERT(volume >= 0 && volume <= 100 && "Volume must be a number between 0 and 100");

	if (!m_AudioBuffer)
	{
		return;
	}

	m_AudioBuffer->VPlay(volume, looping);
}

void SoundProcess::Stop()
{
	if (m_AudioBuffer) {
		m_AudioBuffer->VStop();
	}
}

float SoundProcess::GetProgress()
{
	if (m_AudioBuffer)
	{
		return m_AudioBuffer->VGetProgress();
	}

	return 0.0f;
}
