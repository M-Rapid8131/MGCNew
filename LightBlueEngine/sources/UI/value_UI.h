#ifndef __VALUE_UI_H__
#define __VALUE_UI_H__

// <>インクルード
#include <memory>
#include <vector>

// ""インクルード
// LightBlueEngine
#include "2D/sprite.h"

// class >> [ValueUI]
// UIを管理するクラスの元になるクラス。
class ValueUI
{
private:
	// private:定数
	static constexpr float DEFAULT_FLASH_TIME = 1.0f;

	// private:構造体
	struct Value
	{
		std::vector<int>	ivalue;
		std::vector<float>	fvalue;
	};

	struct ValuePosition
	{
		std::vector<DirectX::XMFLOAT2>	ivalue_position;
		std::vector<DirectX::XMFLOAT2>	fvalue_position;
	};

	// private:定数バッファ構造体
	struct CbUI
	{
		// defineやconst intを用いて名前のようなものをUIごとにつけるため、配列を使用
		int		ui_iparameters[4];
		float	ui_fparameters[4];
	};

public:
	// public:コンストラクタ・デストラクタ
	ValueUI() {};
	~ValueUI() {};

	// public:通常関数
	void Initialize(const wchar_t*, const std::wstring&, const DirectX::XMFLOAT2);
	void Update(float);
	void DebugGUI();
	void Render();

	// public:ゲッター関数
	bool			GetIsSelecting() const { return is_selecting; }
	std::wstring	GetName() const { return name; }
	int				GetIntValue(size_t index)
	{
		if (index < value.ivalue.size())
			return value.ivalue.at(index);
	}

	float			GetFloatValue(size_t index)
	{
		if (index < value.fvalue.size())
			return value.fvalue.at(index);
	}

	// public:セッター関数
	void		SwitchIsSelecting()									{ is_selecting = !is_selecting; }
	void		SetIsSelecting(const bool& set)						{ is_selecting = set; }
	void		SetShakePosition(const DirectX::XMFLOAT2& shake)	{ shake_position = shake; }
	void		SetFlashTime(float flash)							{ flash_time = flash; }
	void		SetUIScale(DirectX::XMFLOAT2 new_scale)				{ scale = new_scale; }
	void		PushIntValue(const int init_value, const DirectX::XMFLOAT2& init_position)
	{
		value.ivalue.push_back(init_value);
		value_position.ivalue_position.push_back(init_position);
	}

	void		PushFloatValue(const float init_value, const DirectX::XMFLOAT2& init_position)
	{
		value.fvalue.push_back(init_value);
		value_position.fvalue_position.push_back(init_position);
	}

	void		SetIntValue(size_t index, int new_value, float flash = DEFAULT_FLASH_TIME)
	{
		if (index < value.ivalue.size())
		{
			value.ivalue.at(index) = new_value;
			SetFlashTime(flash);
		}
	}

	void		SetFloatValue(size_t index, float new_value, float flash = DEFAULT_FLASH_TIME)
	{
		if (index < value.fvalue.size())
		{
			value.fvalue.at(index) = new_value;
			SetFlashTime(flash);
		}
	}

	void		SetIntValuePosition(size_t index, const DirectX::XMFLOAT2& new_position)
	{
		if (index < value.ivalue.size())
			value_position.ivalue_position.at(index) = new_position;
	}

	void		SetFloatValuePosition(size_t index, const DirectX::XMFLOAT2& new_position)
	{
		if (index < value.fvalue.size())
			value_position.fvalue_position.at(index) = new_position;
	}

private:
	// private:変数
	bool						selectable			= false;
	bool						is_selecting		= false;
	float						flash_time			= 0.0f;
	std::wstring				name;
	DirectX::XMFLOAT2			ui_size				= { 0.0f,0.0f };
	DirectX::XMFLOAT2			position			= { 0.0f,0.0f };
	DirectX::XMFLOAT2			scale				= { 1.0f,1.0f };
	DirectX::XMFLOAT2			shake_position		= { 0.0f,0.0f };
	ComPtr<ID3D11Buffer>		UI_cbuffer;
	ComPtr<ID3D11PixelShader>	UI_pixel_shader;
	std::unique_ptr<Sprite>		background_sprite;
	Value						value;
	ValuePosition				value_position;
	CbUI						UI_constant			= {};
};

#endif // __VALUE_UI_H__