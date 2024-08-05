#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

// <>�C���N���[�h
#include <string>
#include <filesystem>

// ""�C���N���[�h
// LightBlueEngine
#include "engine_utility.h"
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
	VOICE,
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
	BONUS_TIME,
	RESULT,

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
	ENTER_GAME,
	CHAIN,
	GAME_CLEAR,
};

enum class EnumVoiceBank
{
	NONE
};

// class >> [AudioManager]
// �����f�[�^���Ǘ�����N���X�BBGM�ASE�A�{�C�X�ɕ��މ\�B
class AudioManager
{
public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	AudioManager() {};
	~AudioManager();

	//public:�R�s�[�R���X�g���N�^�E���[�u�R���X�g���N�^
	AudioManager(const AudioManager&) = delete;
	AudioManager& operator = (const AudioManager&) = delete;
	AudioManager(AudioManager&&) noexcept = delete;
	AudioManager& operator= (AudioManager&&) noexcept = delete;

	// public:�ʏ�֐�
	void PlayBGM(EnumBGMBank, bool, float = 1.0f);
	void StopBGM();
	void PlaySE(EnumSEBank);
	void PlayVoice(EnumVoiceBank);

	// �Q�b�^�[�֐�
	bool IsPlaying() 
	{
		if (playing_bgm != EnumBGMBank::NONE)
			return bgm_bank[SCast(int, playing_bgm)]->GetIsPlaying();
		else
			return false;
	}

	// public:�Z�b�^�[�֐�
	void SetAudioBankFromJSON(Audio*, std::filesystem::path);

private:
	// private:�ϐ�
	EnumBGMBank							playing_bgm = EnumBGMBank::NONE;
	UPtrVector<AudioSource>				bgm_bank;
	UPtrVector<AudioSource>				se_bank;
	UPtrVector<AudioSource>				voice_bank;
};

#endif // __AUDIO_MANAGER_H__