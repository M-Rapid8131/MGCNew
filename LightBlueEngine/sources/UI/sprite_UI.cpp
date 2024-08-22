// ""インクルード
// LightBlueEngine
#include "xmfloat_calclation.h"
#include "sprite_UI.h"

// 初期化処理
void SpriteUI::Initialize(const std::wstring& w_sprite_filename, const std::wstring& w_init_name, const DirectX::XMFLOAT2 init_position)
{
	name = w_init_name;
	position = init_position;
	if(w_sprite_filename != L"")
		background_sprite = std::make_unique<Sprite>(w_sprite_filename.c_str(), "shader/2d_base_ps.cso");
}

// 描画処理
void SpriteUI::Render(float alpha)
{
	DirectX::XMFLOAT2 shaked_position = XMFloatCalclation::XMFloat2Add(position, shake_position);

	DirectX::XMFLOAT4 flash_color;
	flash_color.x = 1.0f;
	flash_color.w = alpha;

	flash_color.y = 1.0f - (flash_time * 0.5f);
	flash_color.z = 1.0f - (flash_time * 0.5f);

	if(background_sprite.get())
		background_sprite->Render(shaked_position,
			background_sprite->GetSpriteSizeWithScaling(scale), flash_color);

	// UIとして使う画像の描画
	size_t index_count = 0;
	for(size_t value : index_value)
	{
		Sprite*				sprite			= sprite_image.at(value).sprite.get();
		DirectX::XMFLOAT2	size				= sprite_image.at(value).size;
		DirectX::XMFLOAT2	sprite_position = image_position.at(index_count);

		sprite->Render(XMFloatCalclation::XMFloat2Add(sprite_position, shaked_position), size, {1.0f, 1.0f, 1.0f, alpha});
		index_count++;
	}
}
