#ifndef __FONT_RENDERER_H__
#define __FONT_RENDERER_H__

// <>�C���N���[�h
#include <d2d1.h>
#include <wrl.h>
#include <DirectXMath.h>

// ""�C���N���[�h
// LightBlueEngine
#include "singleton.h"

// using
using Microsoft::WRL::ComPtr;

// class >> [FontRenderer]
// Direct2D��DirectWrite��p�����e�L�X�g�o�̓V�X�e���B
class FontRenderer
{
public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	FontRenderer(const wchar_t*, DirectX::XMFLOAT4 = { 1.0f,1.0f,1.0f,1.0f });
	~FontRenderer();

	// public:�ʏ�֐�
	void Begin();
	void RenderText(const wchar_t*, DirectX::XMFLOAT4);
	void End();

private:
	// private:�ϐ�
	FLOAT							font_size = 100.0f;
	ComPtr<IDWriteTextFormat>		dwrite_text_format;
	ComPtr<ID2D1SolidColorBrush>	d2d1_solid_brush;
};

#endif // __FONT_RENDERER_H__