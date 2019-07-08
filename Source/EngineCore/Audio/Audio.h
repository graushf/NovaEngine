#pragma once

//========================================================================
// Audio.h : Defines a simple sound system.
//========================================================================

class SoundResourceExtraData;

enum SoundType
{
	SOUND_TYPE_FIRST,
	SOUND_TYPE_MP3 = SOUND_TYPE_FIRST,
	SOUND_TYPE_WAVE,
	SOUND_TYPE_MIDI,
	SOUND_TYPE_OGG,

	// This needs to be the last sound type
	SOUND_TYPE_COUNT,
	SOUND_TYPE_UNKNOWN,
};

//extern char* gSoundExtensions[];
