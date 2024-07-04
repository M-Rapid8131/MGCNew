#ifndef __IMGUI_RENDERER_H__
#define __IMGUI_RENDERER_H__

// <>�C���N���[�h
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <vector>
#include <functional>

// ""�C���N���[�h
// �Q�[���\�[�X�t�@�C��
#include "misc.h"

// define
#define IMGUI_DISABLE_OBSOLETE_KEYIO

// extern�֐�
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// extern�ϐ�
extern ImWchar glyphRangesJapanese[];

// class >> [ImGuiRenderer]
// ImGui�Ɋւ�铮����܂Ƃ߂��N���X�B
class ImGuiRenderer
{
public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	ImGuiRenderer() {}
	ImGuiRenderer(HWND);
	~ImGuiRenderer();

	// public:�ʏ�֐�
	void Begin();
	void Render();
};

#endif // __IMGUI_RENDERER_H__
