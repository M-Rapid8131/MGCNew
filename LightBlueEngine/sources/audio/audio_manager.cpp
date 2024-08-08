// ""インクルード
// LightBlueEngine
#include "engine_utility.h"
#include "json_editor.h"
#include "audio.h"
#include "audio_manager.h"

// ゲームソースファイル
#include "misc.h"

//-------------------------------------------
// AudioManager メンバ関数
//-------------------------------------------
// デストラクタ
AudioManager::~AudioManager()
{
	// BGM全消し
	for (auto itr = bgm_bank.begin(); itr != bgm_bank.end(); itr++)
	{
		auto ptr = (*itr).release();
		delete ptr;
	}
	bgm_bank.clear();
	bgm_bank.shrink_to_fit();

	// SE全消し
	for (auto itr = se_bank.begin(); itr != se_bank.end(); itr++)
	{
		auto ptr = (*itr).release();
		delete ptr;
	}
	se_bank.clear();
	se_bank.shrink_to_fit();

	// ボイス全消し
	for (auto itr = voice_bank.begin(); itr != voice_bank.end(); itr++)
	{
		auto ptr = (*itr).release();
		delete ptr;
	}
	voice_bank.clear();
	voice_bank.shrink_to_fit();
}

// BGM再生
void AudioManager::PlayBGM(EnumBGMBank bgm, bool loop, float volume)
{
	size_t index = SCast(size_t,bgm);
	bgm_bank.at(index)->Stop();
	bgm_bank.at(index)->PlayUsingMediaFoundation(loop, volume);

	playing_bgm = bgm;
}

// BGM停止
void AudioManager::StopBGM()
{
	if (playing_bgm == EnumBGMBank::NONE)	return;
	size_t index = SCast(size_t, playing_bgm);
	bgm_bank.at(index)->Stop();
	playing_bgm = EnumBGMBank::NONE;
}

// SE再生
void AudioManager::PlaySE(EnumSEBank se)
{
	size_t index = SCast(size_t, se);
	se_bank.at(index)->Stop();
	se_bank.at(index)->PlayUsingMediaFoundation(false);
}

// ボイス再生
void AudioManager::PlayVoice(EnumVoiceBank voice)
{
	size_t index = SCast(size_t, voice);
	voice_bank.at(index)->Stop();
	voice_bank.at(index)->PlayUsingMediaFoundation(false);
}

// JSONファイルから音声ファイル読み込み
void AudioManager::SetAudioBankFromJSON(Audio* audio, std::filesystem::path json_path)
{
	std::function<std::wstring(std::string)> stows = [](const std::string str) -> std::wstring {
		wchar_t wc[128];
		mbstowcs_s(nullptr, wc, str.c_str(), sizeof(wc));
		std::wstring wstr;
		wstr.assign(wc);
		return wstr;
	};

	JSONEditor* json_editor = JSONEditor::GetInstance();

	Parameters	aufio_params;
	ParamPtr	bgm_params;
	ParamPtr	se_params;
	ParamPtr	voice_params;

	json_editor->ImportJSON(json_path, &aufio_params);
	bgm_params		= GET_PARAMETER_IN_PARAMPTR("BGM",		Parameters, &aufio_params);
	se_params		= GET_PARAMETER_IN_PARAMPTR("SE",		Parameters, &aufio_params);
	voice_params	= GET_PARAMETER_IN_PARAMPTR("VOICE",	Parameters, &aufio_params);

	if (bgm_params)
	{
		for (size_t i = 0; i < bgm_params->size(); i++)
		{
			std::string audio_path = *(GET_PARAMETER_IN_PARAMPTR(std::to_string(i), std::string, bgm_params));
			auto obj = audio->LoadAudioSourceUsingMF(stows(audio_path));
			bgm_bank.push_back(std::move(obj));
		}
	}

	if (se_params)
	{
		for (size_t i = 0; i < se_params->size(); i++)
		{
			std::string audio_path = *(GET_PARAMETER_IN_PARAMPTR(std::to_string(i), std::string, se_params));
			auto obj = audio->LoadAudioSourceUsingMF(stows(audio_path).c_str());
			se_bank.push_back(std::move(obj));
		}
	}

	if(voice_params)
	{
		for (size_t i = 0; i < voice_params->size(); i++)
		{
			std::string audio_path = *(GET_PARAMETER_IN_PARAMPTR(std::to_string(i), std::string, voice_params));
			auto obj = audio->LoadAudioSourceUsingMF(stows(audio_path).c_str());
			voice_bank.push_back(std::move(obj));
		}
	}
}
