#ifndef __GAMESYSTEM_DIRECTOR_H__
#define __GAMESYSTEM_DIRECTOR_H__

// <>インクルード
#include <memory>

// ""インクルード
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
// FlagSystemに使用
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
	UINT	speed_level = 1;			// 現在のスピードレベル
	UINT	init_level = 1;				// 初期レベル
	UINT	max_level = 50;				// クリアに必要なレベル
	UINT	deleted_block_count = 0;	// 消したブロックの数
	UINT	level_up_block = 0;			// レベルアップに必要なブロック数
	UINT	score = 0;					// スコア
};

// class >> [GamesystemDirector] 継承：Singleton<GamesystemDirector>
// ゲームの様々な環境要素(AudioやGraphicsなど)を保持するクラス。
class GamesystemDirector : public Singleton<GamesystemDirector>
{
public:
	// public:コンストラクタ・デストラクタ
	GamesystemDirector() {};
	~GamesystemDirector() {};

	// public:通常関数
	void Initialize(HWND, EnumCameraMode&);
	void Update(float);
	void DebugGUI();
	void Render();
	void RestoreWindowMode();

	// public:ゲッター関数
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

	// public:セッター関数
	void MakeShadowMap();
	void MakeSkyMap(const wchar_t*);
	void MakeFontRenderer(const wchar_t*, DirectX::XMFLOAT4 = { 1.0f,1.0f,1.0f,1.0f });
	void MakeNumberRenderer();
	void MakeRandomNoise();
	void SetRadialBlur()				{ camera->RadialBlur(); }
	void ClearGameData()				{ saved_game_data.clear(); }
	void SaveGameData(GameData& data)	{ saved_game_data.push_back(data); }

private:
	// private:変数
	// 2D
	std::unique_ptr<FramebufferManager> framebuffer_manager;// 必須

	// 3D
	std::unique_ptr<ShadowMap>			shadow_map;

	// Audio
	std::unique_ptr<Audio>				audio;				// 必須
	std::unique_ptr<AudioManager>		audio_manager;		// 必須

	// Graphics
	std::unique_ptr<Camera>				camera;				// 必須 updateあり
	std::unique_ptr<Light>				light;				// 必須 updateあり
	std::unique_ptr<SkyMap>				sky_map;			// renderあり

	// Scene
	std::unique_ptr<SceneManager>		scene_manager;		// 必須 update、renderあり

	// UI
	std::unique_ptr<FontRenderer>		font_renderer;		// renderあり
	std::unique_ptr<NumberRenderer>		number_renderer;	// renderあり

	// Utility
	std::unique_ptr<ImGuiRenderer>		imgui_renderer;		// 必須
	std::unique_ptr<RandomNoise>		random_noise;

	FlagSystem<EnumUsingClass>			using_class_flag;

	bool								show_imgui = true;
	BOOL								windowed = FALSE;
	std::vector<GameData>				saved_game_data;
};

#endif // __GAMESYSTEM_DIRECTOR_H__