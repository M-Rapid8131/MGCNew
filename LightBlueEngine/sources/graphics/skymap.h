#ifndef __SKYMAP_H__
#define __SKYMAP_H__

// <>インクルード
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

// using
using Microsoft::WRL::ComPtr;

using SkyboxArray = std::vector<ComPtr<ID3D11ShaderResourceView>>;

// enum class
enum class EnumSkyboxTextureID
{
	SKYBOX = 32,
	DIFFUSE_IEM,
	SPECULAR_PMREM,
	LUT_GGX
};

enum class EnumBufferSkymap
{
	SKYMAP,
};

// class >> [SkyMap]
// ゲーム世界の周辺を囲む天球を生成するクラス。
class SkyMap
{
private:
	// private:定数バッファ構造体
	// struct >> SkyMap >> [CbSkymap] register : b0
	struct CbSkymap
	{
		DirectX::XMFLOAT4X4	inv_view_projection;
		int					lod = 0;
	};


public:
	// public:コンストラクタ・デストラクタ
	SkyMap(const wchar_t*);
	virtual ~SkyMap() {};

	// public:通常関数
	void Render(DirectX::XMFLOAT4X4, DirectX::XMFLOAT4X4, int = 0);
	void Render2Cubemap();

	// public:ゲッター関数
	ComPtr<ID3D11ShaderResourceView>		GetSkyboxSRV(EnumSkyboxTextureID id)
	{
		return skymap_srvs.at(SCast(size_t, id) - SCast(size_t, EnumSkyboxTextureID::SKYBOX));
	}

private:
	// private:変数
	bool								is_texturecube		= false;
	CbSkymap							skymap_constant		= {};
	ComPtr<ID3D11VertexShader>			skymap_vs;
	ComPtr<ID3D11PixelShader>			skymap_ps;
	ComPtr<ID3D11PixelShader>			skybox_ps;
	ComPtr<ID3D11VertexShader>			skymap_texturecube_vs;
	ComPtr<ID3D11GeometryShader>		skymap_texturecube_gs;
	ComPtr<ID3D11PixelShader>			skymap_texturecube_ps;
	ComPtr<ID3D11PixelShader>			skybox_texturecube_ps;
	ComPtr<ID3D11Buffer>				skymap_cbuffer;
	
	SkyboxArray							skymap_srvs;
};

#endif // __SKYMAP_H__