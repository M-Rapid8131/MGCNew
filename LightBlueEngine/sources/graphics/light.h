#ifndef __LIGHT_H__
#define __LIGHT_H__

// <>�C���N���[�h
#include <d3d11.h>
#include <windows.h>
#include <tchar.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include <wrl.h>
#include <DirectXMath.h>

// �Q�[���\�[�X�t�@�C��
#include "misc.h"

// using
using Microsoft::WRL::ComPtr;

// �\����
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
// �Q�[�����E�𖾂邭�Ƃ炷���߂̃N���X�B
class Light
{
private:
	// private:�萔�o�b�t�@�\����
	
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
	// public:�R���X�g���N�^�E�f�X�g���N�^
	Light();
	~Light();

	//public:�R�s�[�R���X�g���N�^�E���[�u�R���X�g���N�^
	Light(const Light&)					= delete;
	Light& operator = (const Light&)	= delete;
	Light(Light&&) noexcept				= delete;
	Light& operator= (Light&&) noexcept = delete;

	// public:�ʏ�֐�
	void DebugGUI();
	void Update();

	// public:�Q�b�^�[�֐�
	float		GetSkyColorDest() const { return sky_color_constants.sky_mix_color_dest.x; }
	CbLight&	GetLightConstants()		{ return light_constants; }

	// public:�Z�b�^�[�֐�
	void	 ChangeSkyColorShift(const float dest_color) { sky_color_constants.sky_mix_color_dest.x = dest_color; }

private:
	// private:�ϐ�
	ComPtr<ID3D11Buffer>	sky_color_cbuffer;
	ComPtr<ID3D11Buffer>	light_cbuffer;
	CbSkyColor				sky_color_constants		= {};
	CbLight					light_constants			= {};
};

#endif // __LIGHT_H__