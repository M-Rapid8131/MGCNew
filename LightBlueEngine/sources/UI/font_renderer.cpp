// ""�C���N���[�h
// LightBlueEngine
#include "graphics/graphics.h"
#include "font_renderer.h"

// �Q�[���\�[�X�t�@�C��
#include "misc.h"

//--------------------------------------
// FontRenderer �����o�֐�
//--------------------------------------

// �R���X�g���N�^
FontRenderer::FontRenderer(const wchar_t* w_font_name,DirectX::XMFLOAT4 color)
{
	Graphics2D& graphics_2d = Graphics::GetInstance()->GetGraphics2D();

	HRESULT hr = S_OK;

	// TextFormat�쐬
	hr = graphics_2d.dwrite_factory->CreateTextFormat(
		w_font_name,
		nullptr,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_MEDIUM,
		font_size,
		L"",
		dwrite_text_format.GetAddressOf()
	);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// TextAlignment�ݒ�
	hr = dwrite_text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

	// XMFLOAT4����D2D1_COLOR_F��
	D2D1_COLOR_F d2d1_color;
	d2d1_color.r = color.x;
	d2d1_color.g = color.y;
	d2d1_color.b = color.z;
	d2d1_color.a = color.w;

	// �t�H���g�̐F�ݒ�
	hr = graphics_2d.d2d1_render_target->CreateSolidColorBrush(d2d1_color, d2d1_solid_brush.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

}

// �f�X�g���N�^
FontRenderer::~FontRenderer()
{
	dwrite_text_format.Reset();
	d2d1_solid_brush.Reset();
}

// �`��J�n
void FontRenderer::Begin()
{
	Graphics2D& graphics_2d = Graphics::GetInstance()->GetGraphics2D();
	graphics_2d.d2d1_render_target->BeginDraw();
}

// �e�L�X�g�`��
void FontRenderer::RenderText(const wchar_t* w_text, DirectX::XMFLOAT4 new_color)
{
	Graphics2D& graphics_2d = Graphics::GetInstance()->GetGraphics2D();

	// XMFLOAT4����D2D1_COLOR_F��
	D2D1_COLOR_F d2d1_color;
	d2d1_color.r = new_color.x;
	d2d1_color.g = new_color.y;
	d2d1_color.b = new_color.z;
	d2d1_color.a = new_color.w;

	d2d1_solid_brush->SetColor(d2d1_color);
	graphics_2d.d2d1_render_target->DrawText(
		w_text, SCast(UINT32, wcslen(w_text) - 1),
		dwrite_text_format.Get(), D2D1_RECT_F(0, 0, 1280, 100),
		d2d1_solid_brush.Get(), D2D1_DRAW_TEXT_OPTIONS_NONE
	);
}

// �`��I��
void FontRenderer::End()
{
	Graphics2D& graphics_2d = Graphics::GetInstance()->GetGraphics2D();
	graphics_2d.d2d1_render_target->EndDraw();
}