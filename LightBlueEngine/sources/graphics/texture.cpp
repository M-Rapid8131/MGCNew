// <>�C���N���[�h
#include <mutex>
#include <filesystem>
#include <DirectXTex.h>

// ""�C���N���[�h
// LightBlueEngine
#include "graphics.h"
#include "texture.h"

//// static�����o�ϐ��̏�����
//TextureCache TextureLoader::cached_textures = {};
//
//--------------------------------------------
// TextureLoader �����o�֐�
//--------------------------------------------

// cached_textures��S���폜
void TextureLoader::ReleaseAllTextures()
{
	for (auto& texture : cached_textures)
	{
		texture.second->Release();
		cached_textures.erase(texture.first);
	}
	cached_textures.clear();
}

// mip_level��p���Ȃ��e�N�X�`���ǂݍ���
HRESULT TextureLoader::LoadTextureFromFile(const wchar_t* filename,
	ID3D11ShaderResourceView** shader_resource_view, D3D11_TEXTURE2D_DESC* texture2d_desc)
{
	HRESULT hr = TextureLoader::LoadTextureFromFile(filename, shader_resource_view, texture2d_desc, 0);
	return hr;
}

// mip_level��p����e�N�X�`���ǂݍ���
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

		// DDS�t�@�C���ǂݍ���
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
			// WIC�`���̃t�@�C���ǂݍ���
			if (L".png" == extension || L".jpg" == extension || L".jpeg" == extension || L".gif" == extension)
			{
				hr = DirectX::LoadFromWICFile(path.c_str(), DirectX::WIC_FLAGS::WIC_FLAGS_NONE, &metadata, scratch_image);
				_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			}
			// HDR�t�@�C���ǂݍ���
			else if (L".hdr" == extension)
			{
				hr = DirectX::LoadFromHDRFile(path.c_str(), &metadata, scratch_image);
				_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			}
			else
			{
				_ASSERT_EXPR(false, L"���̉摜�`���̓T�|�[�g���Ă��܂���");
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

	// Texture2D desc���擾
	ComPtr<ID3D11Resource>	resource;
	(*shader_resource_view)->GetResource(resource.ReleaseAndGetAddressOf());

	ComPtr<ID3D11Texture2D> texture2d;
	hr = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	texture2d->GetDesc(texture2d_desc);

	return hr;
}

// cached_textures�ɓ����Ă���e�N�X�`���f�[�^����
// mip_level��p�����ɓǂݍ���
HRESULT TextureLoader::LoadTextureFromMemory(std::wstring name, const void* data, size_t size,
	ID3D11ShaderResourceView** shader_resource_view) 
{
	HRESULT hr = TextureLoader::LoadTextureFromMemory(name, data, size, shader_resource_view, 0);
	return hr;
}

// cached_textures�ɓ����Ă���e�N�X�`���f�[�^����
// mip_level��p���ēǂݍ���
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

		// �e��t�@�C����ǂݍ���
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
			_ASSERT_EXPR(false, L"����������̎擾�Ɏ��s");
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

// �_�~�[�e�N�X�`���𐶐�
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

	// �e�N�X�`�����w�肵���F�œh��Ԃ��Ă���
	size_t texels = dimension * dimension;
	std::unique_ptr<DWORD[]> sysmem = std::make_unique<DWORD[]>(texels);
	for (size_t i = 0; i < texels; ++i) 
		sysmem[i] = value;

	D3D11_SUBRESOURCE_DATA subresource_data = {};
	subresource_data.pSysMem			= sysmem.get();
	subresource_data.SysMemPitch		= sizeof(DWORD) * dimension;

	// Texture2D����
	ComPtr<ID3D11Texture2D> texture2d;
	hr = device->CreateTexture2D(&texture2d_desc, &subresource_data, &texture2d);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// �V�F�[�_�[���\�[�X�r���[����
	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc{};
	shader_resource_view_desc.Format				= texture2d_desc.Format;
	shader_resource_view_desc.ViewDimension			= D3D11_SRV_DIMENSION_TEXTURE2D;
	shader_resource_view_desc.Texture2D.MipLevels	= 1;
	hr = device->CreateShaderResourceView(texture2d.Get(), &shader_resource_view_desc,
		shader_resource_view);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	return hr;
}

// SRV����Texture2D���擾
D3D11_TEXTURE2D_DESC TextureLoader::GetTexture2dDescFromSRV(ID3D11ShaderResourceView* srv)
{
	// ���\�[�X�擾
	ComPtr<ID3D11Resource> resource;
	srv->GetResource(resource.ReleaseAndGetAddressOf());

	ComPtr<ID3D11Texture2D> texture2d;
	HRESULT hr = resource->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// Desc�擾
	D3D11_TEXTURE2D_DESC texture2d_desc;
	texture2d->GetDesc(&texture2d_desc);

	return texture2d_desc;
}