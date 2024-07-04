#ifndef __FONT_RENDERER_H__
#define __FONT_RENDERER_H__

// <>インクルード
#include <d2d1.h>
#include <wrl.h>
#include <DirectXMath.h>

// ""インクルード
// LightBlueEngine
#include "singleton.h"

// using
using Microsoft::WRL::ComPtr;

// class >> [FontRenderer]
// Direct2DとDirectWriteを用いたテキスト出力システム。
class FontRenderer
{
public:
	// public:コンストラクタ・デストラクタ
	FontRenderer(const wchar_t*, DirectX::XMFLOAT4 = { 1.0f,1.0f,1.0f,1.0f });
	~FontRenderer();

	// public:通常関数
	void Begin();
	void RenderText(const wchar_t*, DirectX::XMFLOAT4);
	void End();

private:
	// private:変数
	FLOAT							font_size = 100.0f;
	ComPtr<IDWriteTextFormat>		dwrite_text_format;
	ComPtr<ID2D1SolidColorBrush>	d2d1_solid_brush;
};

#endif // __FONT_RENDERER_H__