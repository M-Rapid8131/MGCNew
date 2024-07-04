#ifndef __IMGUI_RENDERER_H__
#define __IMGUI_RENDERER_H__

// <>インクルード
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <vector>
#include <functional>

// ""インクルード
// ゲームソースファイル
#include "misc.h"

// define
#define IMGUI_DISABLE_OBSOLETE_KEYIO

// extern関数
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// extern変数
extern ImWchar glyphRangesJapanese[];

// class >> [ImGuiRenderer]
// ImGuiに関わる動作をまとめたクラス。
class ImGuiRenderer
{
public:
	// public:コンストラクタ・デストラクタ
	ImGuiRenderer() {}
	ImGuiRenderer(HWND);
	~ImGuiRenderer();

	// public:通常関数
	void Begin();
	void Render();
};

#endif // __IMGUI_RENDERER_H__
