#ifndef __SHADOW_MAP_H__
#define __SHADOW_MAP_H__

// <>�C���N���[�h
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <DirectXMath.h>

// �O���錾
class Camera;
class Light;

// using
using Microsoft::WRL::ComPtr;

// class >> [ShadowMap]
// 3D���f���ɉe��t����N���X�B
class ShadowMap
{
private:
	// private:�萔
	const LONG SHADOW_MAP_WIDTH		= 4096;
	const LONG SHADOW_MAP_HEIGHT	= 4096;

	// private:�萔�o�b�t�@�\����
	// struct >> Shadow
	struct CbShadowMap
	{
		DirectX::XMFLOAT4X4		light_view_projection;
		DirectX::XMFLOAT3		shadow_color		= DirectX::XMFLOAT3(0.3f, 0.3f, 0.3f);
		float					shadow_bias		= 0.008f;
	};

public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	ShadowMap();
	~ShadowMap() {};

	//public:�R�s�[�R���X�g���N�^�E���[�u�R���X�g���N�^
	ShadowMap(const ShadowMap&)					= delete;
	ShadowMap& operator = (const ShadowMap&)	= delete;
	ShadowMap(ShadowMap&&) noexcept				= delete;
	ShadowMap& operator= (ShadowMap&&) noexcept = delete;

	// public:�ʏ�֐�
	void DebugGUI();
	void Clear(Camera*, Light*, float = 1);
	void Activate(D3D11_VIEWPORT*);
	void Deactivate(D3D11_VIEWPORT*);

	// public:�Q�b�^�[�֐�
	bool						GetUsingShadowMap()		{ return using_shadow_map; }
	ID3D11ShaderResourceView*	GetShaderRenderView()	{ return shader_resource_view.Get(); }
	ID3D11ShaderResourceView**	GetPPShaderRenderView()	{ return shader_resource_view.GetAddressOf(); }
	ID3D11SamplerState*			GetSamplerState()		{ return sampler_state.Get(); }
	ID3D11SamplerState**		GetPPSamplerState()		{ return sampler_state.GetAddressOf(); }

private:
	// private:�ϐ�
	bool								using_shadow_map			= false;
	float								shadow_map_drawrect			= 30;
	UINT								viewport_count				= D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	D3D11_VIEWPORT						viewport					= {};
	ComPtr<ID3D11Buffer>				shadow_map_constant_buffer;
	ComPtr<ID3D11VertexShader>			vertex_shader;
	ComPtr<ID3D11InputLayout>			input_layout;
	ComPtr<ID3D11SamplerState>			sampler_state;
	ComPtr<ID3D11ShaderResourceView>	shader_resource_view;
	ComPtr<ID3D11RenderTargetView>		cached_render_target_view;
	ComPtr<ID3D11DepthStencilView>		depth_stencil_view;
	ComPtr<ID3D11DepthStencilView>		cached_depth_stencil_view;
	CbShadowMap							shadow_map_constants		= {};
};

#endif // __SHADOW_MAP_H__