// <>インクルード
#include <mutex>
#include <filesystem>
#include <DirectXTex.h>

// ""インクルード
// LightBlueEngine
#include "graphics.h"
#include "texture.h"

//// staticメンバ変数の初期化
//TextureCache TextureLoader::cached_textures = {};
//
//--------------------------------------------
// TextureLoader メンバ関数
//--------------------------------------------

// cached_texturesを全部削除
void TextureLoader::ReleaseAllTextures()
{
	for (auto& texture : cached_textures)
	{
		texture.second->Release();
		cached_textures.erase(texture.first);
	}
	cached_textures.clear();
}

// mip_levelを用いないテクスチャ読み込み
HRESULT TextureLoader::LoadTextureFromFile(const wchar_t* filename,
	ID3D11ShaderResourceView** shader_resource_view, D3D11_TEXTURE2D_DESC* texture2d_desc)
{
	HRESULT hr = TextureLoader::LoadTextureFromFile(filename, shader_resource_view, texture2d_desc, 0);
	return hr;
}

// mip_levelを用いるテクスチャ読み込み
HRESULT TextureLoader::LoadTextureFromFile(const wchar_t* filename,
	ID3D11ShaderResourceView** shader_resource_view, D3D11_TEXTURE2D_DESC* texture2d_desc,size_t mip_levels)
{
	Graphics*		graphics	= Graphics::GetInstance();
	ID3D11Device*	device		= graphics->GetDevice().Get();

	HRESULT hr = S_OK;
	TextureCache::iterator it = cached_textures.find(filename);

	if (it != cached_textures.end())
	{
		*shader_resource_view = it->second.Get();
		(*shader_resource_view)->AddRef();
	}
	else
	{
		DirectX::TexMetadata	metadata = {};
		DirectX::ScratchImage	scratch_image;

		// DDSファイル読み込み
		std::wstring dds_filename = std::filesystem::path(filename).replace_extension("dds");
		if (std::filesystem::exists(dds_filename.c_str()))
		{
			hr = DirectX::LoadFromDDSFile(dds_filename.c_str(), DirectX::DDS_FLAGS::DDS_FLAGS_NONE, &metadata, scratch_image);
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		else
		{
			std::filesystem::path	path(filename);
			std::wstring				extension = path.extension();
			std::transform(extension.begin(), extension.end(), extension.begin(), ::towlower);
			// WIC形式のファイル読み込み
			if (L".png" == extension || L".jpg" == extension || L".jpeg" == extension || L".gif" == extension)
			{
				hr = DirectX::LoadFromWICFile(path.c_str(), DirectX::WIC_FLAGS::WIC_FLAGS_NONE, &metadata, scratch_image);
				_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			}
			// HDRファイル読み込み
			else if (L".hdr" == extension)
			{
				hr = DirectX::LoadFromHDRFile(path.c_str(), &metadata, scratch_image);
				_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			}
			else
			{
				_ASSERT_EXPR(false, L"その画像形式はサポートしていません");
			}
		}

		const DirectX::Image*	images		= scratch_image.GetImages();
		size_t					image_count = scratch_image.GetImageCount();
		DirectX::ScratchImage	mip_chain;
		if (metadata.mipLevels == 1)
		{
			hr = DirectX::GenerateMipMaps(images, image_count, scratch_image.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, mip_levels, mip_chain);
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			metadata	= mip_chain.GetMetadata();
			images		= mip_chain.GetImages();
			image_count = mip_chain.GetImageCount();
		}

		hr = DirectX::CreateShaderResourceView(device, images, image_count, metadata, shader_resource_view);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		cached_textures.insert(std::make_pair(filename, *shader_resource_view));
	}

	// Texture2D descを取得
	ComPtr<ID3D11Resource>	resource;
	(*shader_resource_view)->GetResource(resource.ReleaseAndGetAddressOf());

	ComPtr<ID3D11Texture2D> texture2d;
	hr = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	texture2d->GetDesc(texture2d_desc);

	return hr;
}

// cached_texturesに入っているテクスチャデータから
// mip_levelを用いずに読み込み
HRESULT TextureLoader::LoadTextureFromMemory(std::wstring name, const void* data, size_t size,
	ID3D11ShaderResourceView** shader_resource_view) 
{
	HRESULT hr = TextureLoader::LoadTextureFromMemory(name, data, size, shader_resource_view, 0);
	return hr;
}

// cached_texturesに入っているテクスチャデータから
// mip_levelを用いて読み込み
HRESULT TextureLoader::LoadTextureFromMemory(std::wstring name, const void* data, size_t size,
	ID3D11ShaderResourceView** shader_resource_view,size_t mip_levels) 
{
	Graphics*		graphics	= Graphics::GetInstance();
	ID3D11Device*	device		= graphics->GetDevice().Get();

	HRESULT					hr = S_OK;
	TextureCache::iterator	it = cached_textures.find(name);

	if (it != cached_textures.end())
	{
		*shader_resource_view = it->second.Get();
		(*shader_resource_view)->AddRef();
	}
	else
	{
		DirectX::TexMetadata	metadata;
		DirectX::ScratchImage	scratch_image;

		// 各種ファイルを読み込み
		hr = DirectX::LoadFromWICMemory(data, size, DirectX::WIC_FLAGS::WIC_FLAGS_NONE, &metadata, scratch_image);
		if (hr != S_OK)
		{
			hr = DirectX::LoadFromHDRMemory(data, size, &metadata, scratch_image);
		}
		if (hr != S_OK)
		{
			hr = DirectX::LoadFromDDSMemory(data, size, DirectX::DDS_FLAGS::DDS_FLAGS_NONE, &metadata, scratch_image);
		}
		if (hr != S_OK)
		{
			_ASSERT_EXPR(false, L"メモリからの取得に失敗");
		}

		const DirectX::Image*	images		= scratch_image.GetImages();
		size_t					image_count = scratch_image.GetImageCount();
		DirectX::ScratchImage	mip_chain;
		if (metadata.mipLevels == 1)
		{
			hr = DirectX::GenerateMipMaps(images, image_count, scratch_image.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, mip_levels, mip_chain);
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			metadata		= mip_chain.GetMetadata();
			images		= mip_chain.GetImages();
			image_count = mip_chain.GetImageCount();
		}

		hr = DirectX::CreateShaderResourceView(device, images, image_count, metadata, shader_resource_view);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		cached_textures.insert(std::make_pair(name, *shader_resource_view));

	}

	return hr;
}

// ダミーテクスチャを生成
HRESULT TextureLoader::MakeDummyTexture(ID3D11ShaderResourceView** shader_resource_view,
	DWORD value/*0xAABBGGRR*/, UINT dimension)
{
	Graphics*		graphics	= Graphics::GetInstance();
	ID3D11Device*	device		= graphics->GetDevice().Get();

	HRESULT hr = S_OK;

	D3D11_TEXTURE2D_DESC texture2d_desc = {};
	texture2d_desc.Width				= dimension;
	texture2d_desc.Height				= dimension;
	texture2d_desc.MipLevels			= 1;
	texture2d_desc.ArraySize			= 1;
	texture2d_desc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
	texture2d_desc.SampleDesc.Count		= 1;
	texture2d_desc.SampleDesc.Quality	= 0;
	texture2d_desc.Usage				= D3D11_USAGE_DEFAULT;
	texture2d_desc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;

	// テクスチャを指定した色で塗りつぶしていく
	size_t texels = dimension * dimension;
	std::unique_ptr<DWORD[]> sysmem = std::make_unique<DWORD[]>(texels);
	for (size_t i = 0; i < texels; ++i) 
		sysmem[i] = value;

	D3D11_SUBRESOURCE_DATA subresource_data = {};
	subresource_data.pSysMem			= sysmem.get();
	subresource_data.SysMemPitch		= sizeof(DWORD) * dimension;

	// Texture2D生成
	ComPtr<ID3D11Texture2D> texture2d;
	hr = device->CreateTexture2D(&texture2d_desc, &subresource_data, &texture2d);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// シェーダーリソースビュー生成
	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc{};
	shader_resource_view_desc.Format				= texture2d_desc.Format;
	shader_resource_view_desc.ViewDimension			= D3D11_SRV_DIMENSION_TEXTURE2D;
	shader_resource_view_desc.Texture2D.MipLevels	= 1;
	hr = device->CreateShaderResourceView(texture2d.Get(), &shader_resource_view_desc,
		shader_resource_view);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	return hr;
}

// SRVからTexture2Dを取得
D3D11_TEXTURE2D_DESC TextureLoader::GetTexture2dDescFromSRV(ID3D11ShaderResourceView* srv)
{
	// リソース取得
	ComPtr<ID3D11Resource> resource;
	srv->GetResource(resource.ReleaseAndGetAddressOf());

	ComPtr<ID3D11Texture2D> texture2d;
	HRESULT hr = resource->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// Desc取得
	D3D11_TEXTURE2D_DESC texture2d_desc;
	texture2d->GetDesc(&texture2d_desc);

	return texture2d_desc;
}