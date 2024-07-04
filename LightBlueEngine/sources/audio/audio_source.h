#ifndef __AUDIO_SOURCE_H__
#define __AUDIO_SOURCE_H__

// <>�C���N���[�h
#include <vector>
#include <xaudio2.h>
#include <WAVFileReader.h>

// class >> [AudioSource]
// �������Đ����邽�߂̃N���X�B
class AudioSource
{
public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	
	// wav�̏ꍇ�̃R���X�g���N�^
	AudioSource(IXAudio2*, DirectX::WAVData);
	AudioSource(IXAudio2*, WAVEFORMATEX*,std::vector<BYTE>);
	~AudioSource();

	// public:�ʏ�֐�
	void Play(bool, float = 1.0f);
	void PlayUsingMediaFoundation(bool, float = 1.0f);
	void Stop();

	// public:�Q�b�^�[�֐�
	bool GetIsPlaying() { return is_playing; }

private:
	// private:�ϐ�
	bool							is_playing		= false;
	DirectX::WAVData				wave_data		= {};
	IXAudio2SourceVoice*			source_voice		= nullptr;

	std::vector<BYTE>				media_data		= {};		// MediaFoundation�g�p���Ɏg�p
};

#endif // __AUDIO_SOURCE_H__