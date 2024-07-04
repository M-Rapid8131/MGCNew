#ifndef __FRAMEWORK_H__
#define __FRAMEWORK_H__

// <>インクルード
#include <vector>
#include <wrl.h>
#include <memory>

// ""インクルード
// LightBlueEngine
#include "input/game_pad.h"

// using
using namespace std;
using namespace Microsoft::WRL;

// class >> [Framework]
class Framework
{
	// private:定数
	CONST HWND hwnd;

	// private:構造体

public:
	// public:コンストラクタ・デストラクタ
	Framework(HWND hwnd) : hwnd(hwnd) {};
	~Framework();

	// public:コピーコンストラクタ・ムーブコンストラクタ
	Framework(const Framework&)					= delete;
	Framework& operator=(const Framework&)		= delete;
	Framework(Framework&&) noexcept				= delete;
	Framework& operator=(Framework&&) noexcept	= delete;

	// public:通常関数
	int					Run();
	LRESULT CALLBACK	HandleMessage(HWND, UINT, WPARAM, LPARAM);
	void				CalculateFrameStats();
	bool				Initialize();
	void				Update(float);
	void				Render();
	bool				Uninitialize();

private:
	// private:変数
	float				operationg_time	= 0.0f;
	uint32_t			frames			= 0;
};

#endif // __FRAMEWORK_H__