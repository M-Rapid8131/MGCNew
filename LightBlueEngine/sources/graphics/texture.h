#ifndef __TEXTURE_H__
#define __TEXTURE_H__

// <>�C���N���[�h
#include <../WICTextureLoader/WICTextureLoader11.h>
#include <../DDSTextureLoader/DDSTextureLoader11.h>
#include <d3d11.h>
#include <wrl.h>
#include <string>
#include <map>
#include <vector>
#include <unordered_map>

// using
using Microsoft::WRL::ComPtr;

using TextureCache = std::map<std::wstring, ComPtr<ID3D11ShaderResourceView>>;

// �ϐ�
static TextureCache cached_textures;

// namespace >> [TextureLoader]
// �e�N�X�`����ǂݍ��񂾂�A�_�~�[�e�N�X�`���𐶐�������ł��閼�O��ԁB
namespace TextureLoader
{
	// �֐�
	void ReleaseAllTextures();

	HRESULT LoadTextureFromFile(const wchar_t*, ID3D11ShaderResourceView**, D3D11_TEXTURE2D_DESC*);

	HRESULT LoadTextureFromFile(const wchar_t*, ID3D11ShaderResourceView**, D3D11_TEXTURE2D_DESC*,size_t);

	HRESULT LoadTextureFromMemory(std::wstring, const void*, size_t, ID3D11ShaderResourceView**);

	HRESULT LoadTextureFromMemory(std::wstring, const void*, size_t, ID3D11ShaderResourceView**, size_t);

	HRESULT MakeDummyTexture(ID3D11ShaderResourceView**,
		DWORD/*0xAABBGGRR*/, UINT);

	D3D11_TEXTURE2D_DESC GetTexture2dDescFromSRV(ID3D11ShaderResourceView*);
};

#endif // __TEXTURE_H__