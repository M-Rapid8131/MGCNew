#ifndef __SPRITE_UI_H__
#define __SPRITE_UI_H__

// <>インクルード
#include <memory>
#include <vector>

// ""インクルード
// LightBlueEngine
#include "2D/sprite.h"
#include "graphics/graphics.h"

// class >> [SpriteUI]
// Spriteの画像を用いたUIを管理するクラスの元になるクラス。
class SpriteUI
{
private:
	// private:定数
	static constexpr float DEFAULT_FLASH_TIME = 1.0f;

	// private:定数
	struct SpriteImage
	{
		std::unique_ptr<Sprite> sprite;
		DirectX::XMFLOAT2		size;
	};
public:
	// public:コンストラクタ・デストラクタ
	SpriteUI() {};
	~SpriteUI() {};

	// public:通常関数
	void Initialize(const std::wstring&, const std::wstring&, const DirectX::XMFLOAT2);
	void Render();

	// public:ゲッター関数
	bool			GetIsSelecting() const	{ return is_selecting; }
	std::wstring	GetName() const			{ return name; }

	// public:セッター関数
	void		SwitchIsSelecting()									{ is_selecting = !is_selecting; }
	void		SetIsSelecting(const bool& set)						{ is_selecting = set; }
	void		SetShakePosition(const DirectX::XMFLOAT2& shake)	{ shake_position = shake; }
	void		SetFlashTime(float flash)							{ flash_time = flash; }
	void		SetUIScale(DirectX::XMFLOAT2 new_scale)				{ scale = new_scale; }
	void		PushIndexValue(size_t init_index_value)
	{
		index_value.emplace_back(init_index_value);
	}

	void		PushImagePosition(DirectX::XMFLOAT2 init_position)
	{
		image_position.emplace_back(init_position);
	}

	void		PushSpriteImage(const wchar_t* w_filename, DirectX::XMFLOAT2 init_size)
	{
		auto& new_sprite_image = sprite_image.emplace_back();
		new_sprite_image.sprite		= std::make_unique<Sprite>(w_filename);
		new_sprite_image.size		= init_size;
	}

	void		SetIndexValue(size_t index, size_t new_index_value)
	{
		if (index < sprite_image.size())
		{
			index_value.at(index) = new_index_value;
		}
	}

private:
	// private:変数
	bool							selectable			= false;
	bool							is_selecting		= false;
	float							flash_time			= 0.0f;
	std::wstring					name;
	DirectX::XMFLOAT2				ui_size				= { 0.0f,0.0f };
	DirectX::XMFLOAT2				position			= { 0.0f,0.0f };
	DirectX::XMFLOAT2				scale				= { 1.0f,1.0f };
	DirectX::XMFLOAT2				shake_position		= { 0.0f,0.0f };
	std::unique_ptr<Sprite>			background_sprite;

	std::vector<size_t>				index_value;
	std::vector<DirectX::XMFLOAT2>	image_position;
	std::vector<SpriteImage>		sprite_image;
};

#endif // __SPRITE_UI_H__