#ifndef __AUDIO_H__
#define __AUDIO_H__

// <>�C���N���[�h
#include <xaudio2.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

// ""�C���N���[�h
#include "audio_source.h"
#include "singleton.h"

// class >> [Audio]
// �����̃f�[�^���Đ��ł���悤�ɓǂݍ��ރN���X�B
class Audio
{
public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	Audio();
	~Audio();

	//public:�R�s�[�R���X�g���N�^�E���[�u�R���X�g���N�^
	Audio(const Audio&) = delete;
	Audio& operator = (const Audio&) = delete;
	Audio(Audio&&) noexcept = delete;
	Audio& operator= (Audio&&) noexcept = delete;

	// public:�ʏ�֐�
	std::unique_ptr<AudioSource>		LoadAudioSource(const wchar_t*);
	std::unique_ptr<AudioSource>		LoadAudioSourceUsingMF(const std::wstring&);

private:
	// private:�ϐ�
	IXAudio2*				xaudio			= nullptr;
	IXAudio2MasteringVoice* mastering_voice = nullptr;
};

#endif // __AUDIO_H__