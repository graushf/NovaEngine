//========================================================================
// SoundResource.cpp : 
//========================================================================

#include "Common/CommonStd.h"

#include "SoundResource.h"
#include "Audio.h"

SoundResourceExtraData::SoundResourceExtraData()
	: m_SoundType(SOUND_TYPE_UNKNOWN),
	m_bInitialized(false),
	m_LengthMilli(0)
{

}

unsigned int WaveResourceLoader::VGetLoadedResourceSize(char* rawBuffer, unsigned int rawSize)
{
	DWORD file = 0;
	DWORD fileEnd = 0;

	DWORD length = 0;
	DWORD type = 0;

	DWORD pos = 0;

	// mmioFOURCC -- converts four chars into a 4 byte integer code.
	// The first 4 bytes of a valid .wav file is 'R', 'I', 'F', 'F'

	type = *((DWORD*)(rawBuffer + pos));
	pos += sizeof(DWORD);
	if (type != mmioFOURCC('R', 'I', 'F', 'F')) {
		return false;
	}

	length = *((DWORD*)(rawBuffer + pos));
	pos += sizeof(DWORD);
	type = *((DWORD*)(rawBuffer + pos));
	pos += sizeof(DWORD);

	// 'W', 'A', 'V', 'E' for a legal .wav file
	if (type != mmioFOURCC('W', 'A', 'V', 'E'))
	{
		return false;
	}

	// Find the end of the file
	fileEnd = length - 4;

	bool copiedBuffer = false;

	// Load the .wav format and the .wav data
	// Note that these blocks can be in either order.
	while (file < fileEnd)
	{
		type = *((DWORD*)(rawBuffer + pos));
		pos += sizeof(DWORD);

		length = *((DWORD*)(rawBuffer + pos));
		pos += sizeof(DWORD);

		switch (type)
		{
			case mmioFOURCC('f', 'a', 'c', 't'):
			{
				//Nv_ASSERT(false && "This wav file is compressed. We don't handle compressed wav at this time");
				break;
			}
			case mmioFOURCC('f', 'm', 't', ' '):
			{
				pos += length;
				break;
			}
			case mmioFOURCC('d', 'a', 't', 'a'):
			{
				return length;
			}
		}

		file += length;

		// Increment the pointer past the block we just read,
		// and make sure the pointer is word aligned.
		if (length & 1)
		{
			++pos;
			++file;
		}
	}

	// If we get to here, the .wav file didn't contain all the right pieces.
	return false;
}

bool WaveResourceLoader::VLoadResource(char* rawBuffer, unsigned int rawSize, std::shared_ptr<ResHandle> handle)
{
	std::shared_ptr<SoundResourceExtraData> extra = std::shared_ptr<SoundResourceExtraData>(Nv_NEW SoundResourceExtraData());
	extra->m_SoundType = SOUND_TYPE_WAVE;
	handle->SetExtra(std::shared_ptr<SoundResourceExtraData>(extra));
	if (!ParseWave(rawBuffer, rawSize, handle))
	{
		return false;
	}
	return true;
}

//
// WaveResourceLoader::ParseWave					- Chapter 13, page 401
//
bool WaveResourceLoader::ParseWave(char* wavStream, size_t bufferLength, std::shared_ptr<ResHandle> handle)
{
	std::shared_ptr<SoundResourceExtraData> extra = static_pointer_cast<SoundResourceExtraData>(handle->GetExtra());
	DWORD file = 0;
	DWORD fileEnd = 0;

	DWORD length = 0;
	DWORD type = 0;

	DWORD pos = 0;

	// mmioFOURCC -- converts four chars into a 4 byte integer code.
	// The first 4 bytes of a valid .wav file is 'R','I','F','F'

	type = *((DWORD*)(wavStream + pos));
	pos += sizeof(DWORD);

	if (type != mmioFOURCC('R', 'I', 'F', 'F')) {
		return false;
	}

	length = *((DWORD*)(wavStream + pos));
	pos += sizeof(DWORD);

	type = *((DWORD*)(wavStream + pos));
	pos += sizeof(DWORD);

	// 'W','A','V','E' for a legal .wav file
	if (type != mmioFOURCC('W', 'A', 'V', 'E')) {
		return false;		// not a WAV
	}

	// Find the end of the file.
	fileEnd = length - 4;

	memset(&extra->m_WavFormatEx, 0, sizeof(WAVEFORMATEX));

	bool copiedBuffer = false;

	// Load the .wav format and the .wav data
	// Note that these blocks can be in either order
	while (file < fileEnd)
	{
		type = *((DWORD*)(wavStream + pos));
		pos += sizeof(DWORD);

		length = *((DWORD*)(wavStream + pos));
		pos += sizeof(DWORD);

		switch (type)
		{
			case mmioFOURCC('f','a','c','t'):
			{
				//Nv_ASSERT(false && "This wav file is compressed. We don't handle compressed wav at this time");
				break;
			}

			case mmioFOURCC('f', 'm', 't', ' '):
			{
				memcpy(&extra->m_WavFormatEx, wavStream + pos, length); 
				pos += length;
				extra->m_WavFormatEx.cbSize = (WORD)length;
				break;
			}

			case mmioFOURCC('d', 'a', 't', 'a'):
			{
				copiedBuffer = true;
				if (length != handle->Size())
				{
					//Nv_ASSERT(0 && _T("Wav resource size does not equal the buffer size"));
					return 0;
				}
				memcpy(handle->WritableBuffer(), wavStream + pos, length);
				pos += length;

				break;
			}
		}

		file += length;

		// If both blocks have been seen, we can return true.
		if (copiedBuffer)
		{
			extra->m_LengthMilli = (handle->Size() * 1000) / extra->GerFormat()->nAvgBytesPerSec;
			return true;
		}

		// Increment the pointer past the block we just read,
		// and make sure the pointer is word aligned.
		if (length && 1)
		{
			++pos;
			++file;
		}
	}

	// If we get to here, the .wav file didn't contain all the right pieces.
	return false;
}

//
//	struct OggMemoryFile						- Chapter 13, page 403
//
struct OggMemoryFile
{
	unsigned char* dataPtr;		// Pointer to the data in memory
	size_t dataSize;			// Size of the data
	size_t dataRead;			// Bytes read so far

	OggMemoryFile(void)
	{
		dataPtr = nullptr;
		dataSize = 0;
		dataRead = 0;
	}
};

//
// VorbisRead									- Chapter 13, page 404
//
size_t VorbisRead(void* data_ptr, size_t byteSize, size_t sizeToRead, void* data_src)
{

}

//
// VorbisSeek									- Chapter 13, page 404
//
//int VorbisSeek(void* data_src, ogg_int64_t offset, int origin)
//{
//
//}

//
// VorbisSeek									- Chapter 13, page 405
//
int VorbisClose(void* src)
{
	// Do nothing - we assume someone else is managing the raw buffer
	return 0;
}

//
// VorbisTell									- Chapter 13, page 405
//
long VorbisTell(void* data_src)
{

}

std::shared_ptr<IResourceLoader> CreateWAVResourceLoader()
{
	return std::shared_ptr<IResourceLoader>(Nv_NEW WaveResourceLoader());
}

std::shared_ptr<IResourceLoader> CreateOGGResourceLoader()
{
	return std::shared_ptr<IResourceLoader>(Nv_NEW OggResourceLoader());
}

unsigned int OggResourceLoader::VGetLoadedResourceSize(char* rawBuffer, unsigned int rawSize)
{

}

bool OggResourceLoader::VLoadResource(char* rawBuffer, unsigned int rawSize, std::shared_ptr<ResHandle> handle)
{

}

//
// OggResourceLoader::ParseOgg										- Chapter 13, page 405
//
bool OggResourceLoader::ParseOgg(char* oggStream, size_t length, std::shared_ptr<ResHandle> handle)
{

}