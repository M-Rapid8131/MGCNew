#ifndef __GAMESYSTEM_DIRECTOR_H__
#define __GAMESYSTEM_DIRECTOR_H__

// <>�C���N���[�h
#include <memory>

// ""�C���N���[�h
// LightBlueEngine
#include "2D/framebuffer_manager.h"
#include "3D/shadow_map.h"
#include "audio/audio.h"
#include "audio/audio_manager.h"
#include "graphics/camera.h"
#include "graphics/light.h"
#include "graphics/skymap.h"
#include "scene/scene_manager.h"
#include "UI/font_renderer.h"
#include "UI/number_renderer.h"
#include "engine_utility.h"
#include "imgui_renderer.h"
#include "random_noise.h"
#include "singleton.h"

// enum
// FlagSystem�Ɏg�p
enum class EnumUsingClass
{
	SHADOW_MAP,
	SKY_MAP,
	FONT_RENDERER,
	NUMBER_RENDERER,
	RANDOM_NOISE,
};

enum class EnumPlayerID
{
	PLAYER_1,
	PLAYER_2,
};

enum class EnumCameraIndex
{
	MAIN_CAMERA,
};

// struct >> [GameData]
struct GameData
{
	bool	cleared = false;
	UINT	speed_level = 1;			// ���݂̃X�s�[�h���x��
	UINT	init_level = 1;				// �������x��
	UINT	max_level = 50;				// �N���A�ɕK�v�ȃ��x��
	UINT	deleted_block_count = 0;	// �������u���b�N�̐�
	UINT	level_up_block = 0;			// ���x���A�b�v�ɕK�v�ȃu���b�N��
	UINT	score = 0;					// �X�R�A
};

// class >> [GamesystemDirector] �p���FSingleton<GamesystemDirector>
// �Q�[���̗l�X�Ȋ��v�f(Audio��Graphics�Ȃ�)��ێ�����N���X�B
class GamesystemDirector : public Singleton<GamesystemDirector>
{
public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	GamesystemDirector() {};
	~GamesystemDirector() {};

	// public:�ʏ�֐�
	void Initialize(HWND, EnumCameraMode&);
	void Update(float);
	void DebugGUI();
	void Render();
	void RestoreWindowMode();

	// public:�Q�b�^�[�֐�
	FramebufferManager* GetFramebufferManager() { return framebuffer_manager.get(); }
	ShadowMap*			GetShadowMap()			{ return shadow_map.get(); }
	Audio*				GetAudio()				{ return audio.get(); }
	AudioManager*		GetAudioManager()		{ return audio_manager.get(); }
	
	Camera*				GetCamera()				{ return camera.get(); }

	Light*				GetLight()				{ return light.get(); }
	SkyMap*				GetSkyMap()				{ return sky_map.get(); }
	SceneManager*		GetSceneManager()		{ return scene_manager.get(); }
	FontRenderer*		GetFontRenderer()		{ return font_renderer.get(); }
	NumberRenderer*		GetNumberRenderer()		{ return number_renderer.get(); }
	RandomNoise*		GetRandomNoise()		{ return random_noise.get(); }

	GameData&			GetGameData(size_t index) { return saved_game_data.at(index); }


	FlagSystem<EnumUsingClass>	GetFlagUsingClass() { return using_class_flag; }

	// public:�Z�b�^�[�֐�
	void MakeShadowMap();
	void MakeSkyMap(const wchar_t*);
	void MakeFontRenderer(const wchar_t*, DirectX::XMFLOAT4 = { 1.0f,1.0f,1.0f,1.0f });
	void MakeNumberRenderer();
	void MakeRandomNoise();
	void SetRadialBlur()				{ camera->RadialBlur(); }
	void ClearGameData()				{ saved_game_data.clear(); }
	void SaveGameData(GameData& data)	{ saved_game_data.push_back(data); }

private:
	// private:�ϐ�
	// 2D
	std::unique_ptr<FramebufferManager> framebuffer_manager;// �K�{

	// 3D
	std::unique_ptr<ShadowMap>			shadow_map;

	// Audio
	std::unique_ptr<Audio>				audio;				// �K�{
	std::unique_ptr<AudioManager>		audio_manager;		// �K�{

	// Graphics
	std::unique_ptr<Camera>				camera;				// �K�{ update����
	std::unique_ptr<Light>				light;				// �K�{ update����
	std::unique_ptr<SkyMap>				sky_map;			// render����

	// Scene
	std::unique_ptr<SceneManager>		scene_manager;		// �K�{ update�Arender����

	// UI
	std::unique_ptr<FontRenderer>		font_renderer;		// render����
	std::unique_ptr<NumberRenderer>		number_renderer;	// render����

	// Utility
	std::unique_ptr<ImGuiRenderer>		imgui_renderer;		// �K�{
	std::unique_ptr<RandomNoise>		random_noise;

	FlagSystem<EnumUsingClass>			using_class_flag;

	bool								show_imgui = true;
	BOOL								windowed = FALSE;
	std::vector<GameData>				saved_game_data;
};

#endif // __GAMESYSTEM_DIRECTOR_H__