// ""�C���N���[�h
// LightBlueEngine
#include "audio_source.h"
#include "engine_utility.h"

// �Q�[���\�[�X�t�@�C��
#include "misc.h"

//-------------------------------------------
// AudioSource �����o�֐�
//-------------------------------------------

// �R���X�g���N�^(wave_data)
AudioSource::AudioSource(IXAudio2* xaudio, DirectX::WAVData wave_data) : wave_data(wave_data)
{
	HRESULT hr = S_OK;
	hr = xaudio->CreateSourceVoice(&source_voice, wave_data.wfx);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

// �R���X�g���N�^(media_data)
AudioSource::AudioSource(IXAudio2* xaudio, WAVEFORMATEX* wfx, std::vector<BYTE> media_data) : media_data(media_data)
{
	HRESULT hr = S_OK;
	hr = xaudio->CreateSourceVoice(&source_voice, wfx);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

// �f�X�g���N�^
AudioSource::~AudioSource()
{
	if (source_voice != nullptr)
	{
		source_voice->DestroyVoice();
		source_voice = nullptr;
	}
}

// �Đ�
void AudioSource::Play(bool loop,float volume)
{
	Stop();

	// �o�b�t�@
	XAUDIO2_BUFFER buffer = { 0 };
	buffer.AudioBytes	= wave_data.audioBytes;
	buffer.pAudioData	= wave_data.startAudio;
	buffer.LoopCount		= loop ? XAUDIO2_LOOP_INFINITE : 0;
	buffer.Flags			= XAUDIO2_END_OF_STREAM;

	is_playing = true;

	// �o�b�t�@����̓L���[�ɒǉ�
	source_voice->SubmitSourceBuffer(&buffer);

	// �Đ�
	HRESULT hr = source_voice->Start();
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	source_voice->SetVolume(volume);
}

// MediaFoundation���g���čĐ�
void AudioSource::PlayUsingMediaFoundation(bool loop,float volume)
{
	Stop();

	// �o�b�t�@
	XAUDIO2_BUFFER buffer = { 0 };
	buffer.AudioBytes	= sizeof(BYTE) * SCast(UINT32,media_data.size());
	buffer.pAudioData	= media_data.data();
	buffer.LoopCount		= loop ? XAUDIO2_LOOP_INFINITE : 0;
	buffer.Flags			= XAUDIO2_END_OF_STREAM;

	is_playing = true;

	// �o�b�t�@����̓L���[�ɒǉ�
	source_voice->SubmitSourceBuffer(&buffer);

	// �Đ�
	HRESULT hr = source_voice->Start();
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	source_voice->SetVolume(volume);
}

// ��~
void AudioSource::Stop()
{
	is_playing = false;
	source_voice->FlushSourceBuffers();
	source_voice->Stop();
}