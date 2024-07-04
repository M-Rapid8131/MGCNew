#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

// <>�C���N���[�h
#include <string>
#include <filesystem>

// ""�C���N���[�h
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
	void PlayBGM(EnumBGMBank,float = 1.0f);
	void StopBGM();
	void PlaySE(EnumSEBank);
	void PlayVoice(EnumVoiceBank);

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