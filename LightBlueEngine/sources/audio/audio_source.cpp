// ""インクルード
// LightBlueEngine
#include "audio_source.h"
#include "engine_utility.h"

// ゲームソースファイル
#include "misc.h"

//-------------------------------------------
// AudioSource メンバ関数
//-------------------------------------------

// コンストラクタ(wave_data)
AudioSource::AudioSource(IXAudio2* xaudio, DirectX::WAVData wave_data) : wave_data(wave_data)
{
	HRESULT hr = S_OK;
	hr = xaudio->CreateSourceVoice(&source_voice, wave_data.wfx);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

// コンストラクタ(media_data)
AudioSource::AudioSource(IXAudio2* xaudio, WAVEFORMATEX* wfx, std::vector<BYTE> media_data) : media_data(media_data)
{
	HRESULT hr = S_OK;
	hr = xaudio->CreateSourceVoice(&source_voice, wfx);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

// デストラクタ
AudioSource::~AudioSource()
{
	if (source_voice != nullptr)
	{
		source_voice->DestroyVoice();
		source_voice = nullptr;
	}
}

// 再生
void AudioSource::Play(bool loop,float volume)
{
	Stop();

	// バッファ
	XAUDIO2_BUFFER buffer = { 0 };
	buffer.AudioBytes	= wave_data.audioBytes;
	buffer.pAudioData	= wave_data.startAudio;
	buffer.LoopCount		= loop ? XAUDIO2_LOOP_INFINITE : 0;
	buffer.Flags			= XAUDIO2_END_OF_STREAM;

	is_playing = true;

	// バッファを入力キューに追加
	source_voice->SubmitSourceBuffer(&buffer);

	// 再生
	HRESULT hr = source_voice->Start();
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	source_voice->SetVolume(volume);
}

// MediaFoundationを使って再生
void AudioSource::PlayUsingMediaFoundation(bool loop,float volume)
{
	Stop();

	// バッファ
	XAUDIO2_BUFFER buffer = { 0 };
	buffer.AudioBytes	= sizeof(BYTE) * SCast(UINT32,media_data.size());
	buffer.pAudioData	= media_data.data();
	buffer.LoopCount		= loop ? XAUDIO2_LOOP_INFINITE : 0;
	buffer.Flags			= XAUDIO2_END_OF_STREAM;

	is_playing = true;

	// バッファを入力キューに追加
	source_voice->SubmitSourceBuffer(&buffer);

	// 再生
	HRESULT hr = source_voice->Start();
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	source_voice->SetVolume(volume);
}

// 停止
void AudioSource::Stop()
{
	is_playing = false;
	source_voice->FlushSourceBuffers();
	source_voice->Stop();
}