#ifndef __LIGHT_H__
#define __LIGHT_H__

// <>インクルード
#include <d3d11.h>
#include <windows.h>
#include <tchar.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include <wrl.h>
#include <DirectXMath.h>

// ゲームソースファイル
#include "misc.h"

// using
using Microsoft::WRL::ComPtr;

// 構造体
// struct >> Light >> [PointLight]
struct PointLight
{
	DirectX::XMFLOAT4		position	= { 0,0,0,0 };
	DirectX::XMFLOAT4		color		= { 1,1,1,1 };
	float					range		= 0;
	float					power		= 1.0f;

	float					point_light_fpad[2];
};

// struct >> Light >> [SpotLight]
struct SpotLight
{
	DirectX::XMFLOAT4		position	= { 0,0,0,0 };
	DirectX::XMFLOAT4		direction	= { 0,0,1,0 };
	DirectX::XMFLOAT4		color		= { 1,1,1,1 };
	float					range		= 0;
	float					power		= 1.0f;
	float					inner_corn	= 0.99f;
	float					outer_corn	= 0.9f;
};

// class >> [Light]
// ゲーム世界を明るく照らすためのクラス。
class Light
{
private:
	// private:定数バッファ構造体
	
	// struct >> Light >> [CbLight] register : b10
	struct CbLight
	{
		DirectX::XMFLOAT4		ambient_color				= { 0.2f,0.2f,0.2f,0.2f };
		DirectX::XMFLOAT4		directional_light_color		= { 1.0f,1.0f,1.0f,1.0f };
		DirectX::XMFLOAT3		directional_light_direction = { 0.0f,0.0f,-1.0f };
		float					radiance					= 1.0f;
		DirectX::XMFLOAT3		directional_light_focus		= { 0.0f,0.0f,0.0f };

		float					cblight_fpad;
	};

	// struct >> Light >> [CbSkyColor] register : b11
	struct CbSkyColor
	{
		DirectX::XMFLOAT3	sky_mix_color_base = { 0.0f,1.0f,1.0f };
		DirectX::XMFLOAT3	sky_mix_color_dest = { 0.5f,1.0f,1.0f };

		float				cbsky_color_fpad[2];
	};

public:
	// public:コンストラクタ・デストラクタ
	Light();
	~Light();

	//public:コピーコンストラクタ・ムーブコンストラクタ
	Light(const Light&)					= delete;
	Light& operator = (const Light&)	= delete;
	Light(Light&&) noexcept				= delete;
	Light& operator= (Light&&) noexcept = delete;

	// public:通常関数
	void DebugGUI();
	void Update();

	// public:ゲッター関数
	float		GetSkyColorDest() const { return sky_color_constants.sky_mix_color_dest.x; }
	CbLight&	GetLightConstants()		{ return light_constants; }

	// public:セッター関数
	void	 ChangeSkyColorShift(const float dest_color) { sky_color_constants.sky_mix_color_dest.x = dest_color; }

private:
	// private:変数
	ComPtr<ID3D11Buffer>	sky_color_cbuffer;
	ComPtr<ID3D11Buffer>	light_cbuffer;
	CbSkyColor				sky_color_constants		= {};
	CbLight					light_constants			= {};
};

#endif // __LIGHT_H__