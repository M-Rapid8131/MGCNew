#ifndef __AUDIO_H__
#define __AUDIO_H__

// <>インクルード
#include <xaudio2.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

// ""インクルード
#include "audio_source.h"
#include "singleton.h"

// class >> [Audio]
// 音源のデータを再生できるように読み込むクラス。
class Audio
{
public:
	// public:コンストラクタ・デストラクタ
	Audio();
	~Audio();

	//public:コピーコンストラクタ・ムーブコンストラクタ
	Audio(const Audio&) = delete;
	Audio& operator = (const Audio&) = delete;
	Audio(Audio&&) noexcept = delete;
	Audio& operator= (Audio&&) noexcept = delete;

	// public:通常関数
	std::unique_ptr<AudioSource>		LoadAudioSource(const wchar_t*);
	std::unique_ptr<AudioSource>		LoadAudioSourceUsingMF(const std::wstring&);

private:
	// private:変数
	IXAudio2*				xaudio			= nullptr;
	IXAudio2MasteringVoice* mastering_voice = nullptr;
};

#endif // __AUDIO_H__