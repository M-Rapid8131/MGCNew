// ""インクルード
// LightBlueEngine
#include "game_system/gamesystem_director.h"
#include "xmfloat_calclation.h"
#include "value_UI.h"

// 初期化処理
void ValueUI::Initialize(const wchar_t* w_sprite_filename, const std::wstring& w_init_name, const DirectX::XMFLOAT2 init_position)
{
	name		= w_init_name;
	position	= init_position;
	if(w_sprite_filename)
		background_sprite = std::make_unique<Sprite>(w_sprite_filename, "shader/2d_base_ps.cso");
}

// 更新処理
void ValueUI::Update(float elapsed_time)
{
	if (flash_time > 0.0f)	flash_time -= elapsed_time;
	else					flash_time = 0.0f;
}

// ImGui表示
void ValueUI::DebugGUI()
{
}

// 描画処理
void ValueUI::Render()
{
	NumberRenderer* number_renderer = GamesystemDirector::GetInstance()->GetNumberRenderer();
	DirectX::XMFLOAT2 shaked_position = XMFloatCalclation::XMFloat2Add(position, shake_position);

	DirectX::XMFLOAT4 flash_color;
	flash_color.x = 1.0f;
	flash_color.w = 1.0f;

	flash_color.y = 1.0f - (flash_time * 0.5f);
	flash_color.z = 1.0f - (flash_time * 0.5f);
	if(background_sprite.get())
		background_sprite->Render(shaked_position,
			background_sprite->GetSpriteSizeWithScaling(scale), flash_color);

	const DirectX::XMFLOAT2 TEXT_SIZE = { 0.3f,0.3f };
	// 数値の描画
	for(size_t value_index = 0; value_index < value.ivalue.size(); value_index++)
	{
		const int VALUE = value.ivalue.at(value_index);
		const DirectX::XMFLOAT2 VALUE_POSITION = value_position.ivalue_position.at(value_index);
		DirectX::XMFLOAT2 shaked_value_position = XMFloatCalclation::XMFloat2Add(VALUE_POSITION, shaked_position);
		number_renderer->RenderInt(VALUE, shaked_value_position, TEXT_SIZE, EnumNumberAlignment::RIGHT_ALIGNMENT);
	}
}
