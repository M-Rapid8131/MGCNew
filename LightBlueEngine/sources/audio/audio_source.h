#ifndef __AUDIO_SOURCE_H__
#define __AUDIO_SOURCE_H__

// <>インクルード
#include <vector>
#include <xaudio2.h>
#include <WAVFileReader.h>

// class >> [AudioSource]
// 音源を再生するためのクラス。
class AudioSource
{
public:
	// public:コンストラクタ・デストラクタ
	
	// wavの場合のコンストラクタ
	AudioSource(IXAudio2*, DirectX::WAVData);
	AudioSource(IXAudio2*, WAVEFORMATEX*,std::vector<BYTE>);
	~AudioSource();

	// public:通常関数
	void Play(bool, float = 1.0f);
	void PlayUsingMediaFoundation(bool, float = 1.0f);
	void Stop();

	// public:ゲッター関数
	bool GetIsPlaying() { return is_playing; }

private:
	// private:変数
	bool							is_playing		= false;
	DirectX::WAVData				wave_data		= {};
	IXAudio2SourceVoice*			source_voice		= nullptr;

	std::vector<BYTE>				media_data		= {};		// MediaFoundation使用時に使用
};

#endif // __AUDIO_SOURCE_H__