// <>インクルード
#include <WAVFileReader.h>

// ""インクルード
// LightBlueEngine
#include "engine_utility.h"
#include "audio.h"

// ゲームソースファイル
#include "misc.h"

//-------------------------------------------
// Audio メンバ関数
//-------------------------------------------

// コンストラクタ
Audio::Audio()
{
	HRESULT hr = S_OK;

	UINT32 create_flags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	create_flags |= XAUDIO2_DEBUG_ENGINE;
#endif

	hr = XAudio2Create(&xaudio, create_flags);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = xaudio->CreateMasteringVoice(&mastering_voice);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

}

// デストラクタ
Audio::~Audio()
{
	if (mastering_voice != nullptr)
	{
		mastering_voice->DestroyVoice();
		mastering_voice = nullptr;
	}

	if (xaudio != nullptr)
	{
		xaudio->Release();
		xaudio = nullptr;
	}
}

// 音声ファイル読み込み
std::unique_ptr<AudioSource> Audio::LoadAudioSource(const wchar_t* w_filename)
{
	std::unique_ptr<uint8_t[]>	wave_file = nullptr;
	DirectX::WAVData				wave_data;

	HRESULT hr = DirectX::LoadWAVAudioFromFileEx(w_filename, wave_file, wave_data);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	return std::make_unique<AudioSource>(xaudio,wave_data);
}

// MediaFoundationを使って音声ファイル読み込み
std::unique_ptr<AudioSource> Audio::LoadAudioSourceUsingMF(const std::wstring& w_filename)
{
	HRESULT hr = S_OK;

	WAVEFORMATEX* wfx = nullptr;

	// MediaFoundation関係の処理
	hr = CoInitialize(nullptr);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// ソースリーダー作成
	IMFSourceReader* mf_source_reader = nullptr;
	hr = MFCreateSourceReaderFromURL(w_filename.c_str(), NULL, &mf_source_reader);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// メディアタイプ作成
	IMFMediaType* mf_media_type = nullptr;
	hr = MFCreateMediaType(&mf_media_type);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = mf_media_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = mf_media_type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = mf_source_reader->SetCurrentMediaType(SCast(DWORD, MF_SOURCE_READER_FIRST_AUDIO_STREAM), nullptr, mf_media_type);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	mf_media_type->Release();
	mf_media_type = nullptr;
	hr = mf_source_reader->GetCurrentMediaType(SCast(DWORD, MF_SOURCE_READER_FIRST_AUDIO_STREAM), &mf_media_type);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = MFCreateWaveFormatExFromMFMediaType(mf_media_type, &wfx, nullptr);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// データ読み込み
	std::vector<BYTE> media_data;
	while (true)
	{
		IMFSample*	mf_sample = nullptr;
		DWORD		stream_flags = 0;

		mf_source_reader->ReadSample(SCast(DWORD, MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0,
			nullptr, &stream_flags, nullptr, &mf_sample);

		if (stream_flags & MF_SOURCE_READERF_ENDOFSTREAM)
			break;

		IMFMediaBuffer* mf_media_buffer = nullptr;
		mf_sample->ConvertToContiguousBuffer(&mf_media_buffer);

		BYTE* buffer				= nullptr;
		DWORD current_length		= 0;
		mf_media_buffer->Lock(&buffer, nullptr, &current_length);

		media_data.resize(media_data.size() + current_length);
		memcpy(media_data.data() + media_data.size() - current_length, buffer, current_length);

		mf_media_buffer->Unlock();

		mf_media_buffer->Release();
		mf_sample->Release();
	}

	return std::make_unique<AudioSource>(xaudio,wfx,media_data);
}