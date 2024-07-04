#ifndef __UI_BASE_H__
#define __UI_BASE_H__

// <>インクルード
#include <memory>

// ""インクルード
// LightBlueEngine
#include "2D/sprite.h"

// inline関数
inline DirectX::XMFLOAT2 Rotate(DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 center, float angle)
{
	DirectX::XMVECTOR v_center		= DirectX::XMLoadFloat2(&center);
	DirectX::XMVECTOR v_angle_3d		= {0.0f,0.0f,angle};
	DirectX::XMVECTOR v_pos_3d		= { pos.x,pos.y,0.0f };

	v_pos_3d = DirectX::XMVectorSubtract(v_pos_3d, v_center);

	DirectX::XMMATRIX m_angle_3d = DirectX::XMMatrixRotationRollPitchYawFromVector(v_angle_3d);
	v_pos_3d = DirectX::XMVector3Transform(v_pos_3d, m_angle_3d);

	v_pos_3d = DirectX::XMVectorAdd(v_pos_3d, v_center);

	DirectX::XMStoreFloat2(&pos, v_pos_3d);

	return pos;
};

// class >> [UI]
// UIを管理するクラスの元になるクラス。
class UI
{
protected:
	// private:定数
	static constexpr float FLASH_TIME_DEFAULT = 0.5f;

	// protected:構造体
	struct CbUI
	{
		// defineやconst intを用いて名前のようなものをUIごとにつけるため、
		// 配列を使用
		int		ui_iparameters[4];
		float	ui_fparameters[4];
	};

public:
	// public:コンストラクタ・デストラクタ
	UI(const char* name, DirectX::XMFLOAT2 position) : name(name), position(position) {};
	virtual ~UI()	{};

	// public:通常関数
	virtual void Initialize(const char* name, DirectX::XMFLOAT2 position)	= 0;
	virtual void Update(float elapsed_time) 
	{ 
		if (flash_time > 0.0f)	flash_time -= elapsed_time;
		else					flash_time = 0.0f;
	}
	virtual void Render()	= 0;
	virtual void DebugGUI()	= 0;

	// public:ゲッター関数
	bool		GetIsSelecting()		{ return is_selecting; }
	const char* GetName()			{ return name; }

	// public:セッター関数
	void			SwitchIsSelecting()							{ is_selecting = !is_selecting; }
	void			SetIsSelecting(bool set)						{ is_selecting = set; }
	void			SetShakePosition(DirectX::XMFLOAT2 shake)	{ shake_position = shake; }

protected:
	// private:変数
	bool						selectable		= false;
	bool						is_selecting	= false;
	float						flash_time		= 0.0f;
	const char*					name;
	DirectX::XMFLOAT2			ui_size			= { 0.0f,0.0f };
	DirectX::XMFLOAT2			position		= { 0.0f,0.0f };
	DirectX::XMFLOAT2			scale			= { 1.0f,1.0f };
	DirectX::XMFLOAT2			shake_position	= { 0.0f,0.0f };
	ComPtr<ID3D11Buffer>			UI_cbuffer;
	ComPtr<ID3D11PixelShader>	UI_pixel_shader;
	CbUI						UI_constant		= {};
};

#endif // __UI_BASE_H__