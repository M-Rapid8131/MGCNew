#ifndef __CAMERA_H__
#define __CAMERA_H__

// <>インクルード
#include <vector>
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>

// ""インクルード
#include "json_editor.h"

// enum class
enum class EnumCameraMode
{
	FIRST_PERSON_VIEW,
	THIRD_PERSON_VIEW,

	CAMERA_MODE_NUM
};

// using
using Microsoft::WRL::ComPtr;

// 定数
static const DirectX::XMFLOAT3	DEFAULT_FPV_POSITION	= { 0.0f, 0.0f, 0.0f };
static const DirectX::XMFLOAT4	DEFAULT_FPV_DIRECTION	= { 0.0f, 0.0f, 1.0f, 0.0f };

static const DirectX::XMFLOAT3	DEFAULT_TPV_TARGET		= { 0.0f, 0.0f, 0.0f };
static const DirectX::XMFLOAT4	DEFAULT_TPV_DIRECTION	= { 0.0f, 0.0f, 1.0f, 0.0f };
static const float				DEFAULT_TPV_DISTANCE	= 1.0f;

// class >> [Camera]
class Camera
{
private:
	// private:定数
	static constexpr float BLUR_SIZE = 1.0f;

	// private:定数バッファ構造体
	// struct >> Camera >> [CbScene] register : b9
	struct CbScene
	{
		int						blur_lod = 10;

		int						cbscene_ipad[3];

		DirectX::XMFLOAT4X4		view;
		DirectX::XMFLOAT4X4		projection;
		DirectX::XMFLOAT3		camera_position;
		float					blur_strength = 50.0f;
		float					blur_size = 0.0f;

		float					cbscene_fpad[3];
	};

public:
	// public:構造体
	struct FPVData
	{
		DirectX::XMFLOAT3	fpv_position	= DEFAULT_FPV_POSITION;
		DirectX::XMFLOAT4	fpv_direction	= DEFAULT_FPV_DIRECTION;
	};

	struct TPVData
	{
		DirectX::XMFLOAT3	tpv_target		= DEFAULT_TPV_TARGET;
		DirectX::XMFLOAT4	tpv_direction	= DEFAULT_TPV_DIRECTION;
		float				tpv_distance	= DEFAULT_TPV_DISTANCE;
	};

	// public:コンストラクタ・デストラクタ
	Camera(EnumCameraMode&);
	~Camera() {};

	// public:通常関数
	void Update();
	void DebugGUI();

	// public:ゲッター関数
	DirectX::XMFLOAT4X4&	GetView()			{ return view; }
	DirectX::XMFLOAT4X4&	GetProjection()		{ return projection; }
	ComPtr<ID3D11Buffer>&	GetSceneCBuffer()	{ return scene_cbuffer; }	// shadow_map用
	CbScene&				GetSceneConstants() { return scene_constants; }	// shadow_map用
	FPVData*				GetFPVCamera(size_t channel = 0) { return channel < fpv_data.size() ? &fpv_data[channel] : nullptr; }
	TPVData*				GetTPVCamera(size_t channel = 0) { return channel < tpv_data.size() ? &tpv_data[channel] : nullptr; }

	// public:セッター関数
	void SetView(const DirectX::XMFLOAT4X4 fm_view) { view = fm_view; }
	void SetView(const DirectX::XMMATRIX m_view) { DirectX::XMStoreFloat4x4(&view, m_view); }
	void SetProjection(const DirectX::XMFLOAT4X4 fm_projection)
	{
		projection = fm_projection;
	}

	void SetProjection(const DirectX::XMMATRIX m_projection)
	{
		DirectX::XMStoreFloat4x4(&projection, m_projection);
	}

	void AddFPVCamera(FPVData* = nullptr);
	void AddTPVCamera(TPVData* = nullptr);
	void SetCameraShift(const DirectX::XMFLOAT3 shift)	{ camera_shift = shift; }
	void RadialBlur()									{ scene_constants.blur_size = BLUR_SIZE; }
	void SetFPVChannel(size_t channel)					
	{
		if (fpv_data.size() > channel)
		{
			fpv_channel = channel;
		}
	}

	void SetTPVChannel(size_t channel)					
	{
		if (tpv_data.size() > channel)
		{
			tpv_channel = channel;
		}
	}

private:
	// private:変数
	float					fov				= 40.0f;
	float					near_z			= 0.01f;
	float					far_z			= 1000.0f;
	size_t					fpv_channel		= 0;
	size_t					tpv_channel		= 0;
	DirectX::XMFLOAT3		camera_position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3		camera_focus	= { 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3		camera_shift	= { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3		up				= { 0.0f, -1.0f, 0.0f };
	DirectX::XMFLOAT3		front			= { 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3		right			= { 1.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4X4		scale			= {};
	DirectX::XMFLOAT4X4		rotation		= {};
	DirectX::XMFLOAT4X4		translation		= {};

	DirectX::XMFLOAT4X4	view = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1,
	};

	DirectX::XMFLOAT4X4 projection = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1,
	};
	ComPtr<ID3D11Buffer>	scene_cbuffer;
	CbScene					scene_constants = {};
	EnumCameraMode			camera_mode		= EnumCameraMode::FIRST_PERSON_VIEW;

	std::vector<FPVData>	fpv_data;
	std::vector<TPVData>	tpv_data;
	Parameters				automation_camera_data;
};

#endif // __CAMERA_H__