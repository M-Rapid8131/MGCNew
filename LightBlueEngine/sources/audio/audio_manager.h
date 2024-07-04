#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

// <>インクルード
#include <string>
#include <filesystem>

// ""インクルード
// LightBlueEngine
#include "singleton.h"
#include "audio.h"

// template using
template<typename T>
using UPtrVector = std::vector<std::unique_ptr<T>>;

// enum
enum class EnumBankSelect
{
	BGM,
	SE,
	VOICE
};

enum class EnumBGMBank
{
	LOW_SPEED1,
	LOW_SPEED2,
	HI_SPEED1,
	HI_SPEED2,
	HI_SPEED3,
	IMPACT_SPEED1,
	IMPACT_SPEED2,
	STANDBY,

	NONE
};

enum class EnumSEBank
{
	COUNT_DOWN,
	GAME_START,
	LEVEL_UP,
	STAND,
	ROTATE,
	PUT,
	ERASE,
};

enum class EnumVoiceBank
{
	COUNT_DOWN,
	LEVEL_UP,
	LEVEL,
	TEN,
	TWENTY,
	THIRTY,
	FOURTY,
	FIFTY,
	SIXTY,
	SEVENTY,
	EIGHTY,
	NINETY,
};

// class >> [AudioManager]
// 音源データを管理するクラス。BGM、SE、ボイスに分類可能。
class AudioManager
{
public:
	// public:コンストラクタ・デストラクタ
	AudioManager() {};
	~AudioManager();

	//public:コピーコンストラクタ・ムーブコンストラクタ
	AudioManager(const AudioManager&) = delete;
	AudioManager& operator = (const AudioManager&) = delete;
	AudioManager(AudioManager&&) noexcept = delete;
	AudioManager& operator= (AudioManager&&) noexcept = delete;

	// public:通常関数
	void PlayBGM(EnumBGMBank,float = 1.0f);
	void StopBGM();
	void PlaySE(EnumSEBank);
	void PlayVoice(EnumVoiceBank);

	// public:セッター関数
	void SetAudioBankFromJSON(Audio*, std::filesystem::path);

private:
	// private:変数
	EnumBGMBank							playing_bgm = EnumBGMBank::NONE;
	UPtrVector<AudioSource>				bgm_bank;
	UPtrVector<AudioSource>				se_bank;
	UPtrVector<AudioSource>				voice_bank;
};

#endif // __AUDIO_MANAGER_H__