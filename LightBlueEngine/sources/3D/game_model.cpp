// <>インクルード
#include <functional>
#include <imgui.h>
#include <stack>
#include <string>

// ""インクルード
// LightBlueEngine
#include "graphics/texture.h"
#include "engine_utility.h"
#include "shader.h"

// ゲームソースファイル
#include "misc.h"

// define(tiny_gltf)
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT

#include "game_model.h"

// 関数
// stringをwstringに変換
static const std::wstring ConvertStr2WStr(const std::string str)
{
	const char* chr = str.c_str();
	size_t size;
	mbstowcs_s(&size, nullptr, 0, chr, str.length());

	std::wstring wstr(size, 0);
	mbstowcs_s(&size, &wstr[0], size - 1, chr, str.length());

	return wstr;
}

//--------------------------------------
// GameModel メンバ関数
//--------------------------------------

// コンストラクタ
GameModel::GameModel(const std::string& init_filename)
{
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11Device*			device			= graphics->GetDevice().Get();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	std::lock_guard<std::mutex> lock(graphics->GetMutex());

	bool				succeeded	= false;
	std::string			error, warning;
	tinygltf::Model		gltf_model;
	tinygltf::TinyGLTF	tiny_gltf;

	// filenameの拡張子を判定
	if (init_filename.find(".glb") != std::string::npos)
	{
		succeeded = tiny_gltf.LoadBinaryFromFile(&gltf_model, &error, &warning, init_filename.c_str());
	}
	else if (init_filename.find(".gltf") != std::string::npos)
	{
		succeeded = tiny_gltf.LoadASCIIFromFile(&gltf_model, &error, &warning, init_filename.c_str());
	}

	_ASSERT_EXPR_A(warning.empty(), warning.c_str());
	_ASSERT_EXPR_A(error.empty(),	error.c_str());
	_ASSERT_EXPR_A(succeeded,		L"Failed to load glTF file");

	// scene取得
	for (std::vector<tinygltf::Scene>::const_reference gltf_scene : gltf_model.scenes)
	{
		Scene& scene	= scenes.emplace_back();
		scene.name		= gltf_scene.name;
		scene.nodes		= gltf_scene.nodes;
	}

	// Mesh、Nodes、Material、Textureを読み込む関数
	FetchMeshes(gltf_model);
	FetchNodes(gltf_model);
	FetchMaterials(gltf_model);
	FetchTextures(gltf_model,init_filename);

	std::vector<std::map<std::string, BufferView>> vertex_buffer_views;

	// primitive格納
	for (Mesh& mesh : meshes) 
	{
		for (Mesh::Primitive primitive : mesh.primitives) 
		{
			vertex_buffer_views.emplace_back(primitive.vertex_buffer_views);
		}
	}

	// 頂点バッファ作成とシェーダー読み込み
	for (std::map<std::string, BufferView> vertex_buffer_view : vertex_buffer_views) 
	{
		D3D11_INPUT_ELEMENT_DESC input_element_desc[] = {
			{"POSITION",	0,		vertex_buffer_view.at("POSITION").format,		0,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0},
			{"NORMAL",		0,		vertex_buffer_view.at("NORMAL").format,			1,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0},
			{"TANGENT",		0,		vertex_buffer_view.at("TANGENT").format,		2,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0},
			{"TEXCOORD",	0,		vertex_buffer_view.at("TEXCOORD_0").format,		3,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0},
			{"JOINTS",		0,		vertex_buffer_view.at("JOINTS_0").format,		4,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0},
			{"WEIGHTS",		0,		vertex_buffer_view.at("WEIGHTS_0").format,		5,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0},
		};
		Shader::CreateVSFromCso("shader/3d_base_vs.cso", vertex_shader.ReleaseAndGetAddressOf(),
			input_layout.ReleaseAndGetAddressOf(), input_element_desc, _countof(input_element_desc));
	}
	Shader::CreatePSFromCso("shader/3d_base_ps.cso", pixel_shader.ReleaseAndGetAddressOf());
	
	Shader::CreateCSFromCso("shader/3d_base_initializer_cs.cso", initializer_compute_shader.ReleaseAndGetAddressOf());
	Shader::CreateCSFromCso("shader/3d_base_cs.cso", compute_shader.ReleaseAndGetAddressOf());

	// コンスタントバッファ作成
	D3D11_BUFFER_DESC buffer_desc = {};
	buffer_desc.ByteWidth	= sizeof(CbPrimitive);
	buffer_desc.Usage		= D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags	= D3D11_BIND_CONSTANT_BUFFER;
	HRESULT hr;
	hr = device->CreateBuffer(&buffer_desc, nullptr, primitive_cbuffer.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	buffer_desc = {};
	buffer_desc.ByteWidth	= sizeof(CbObjectColor);
	buffer_desc.Usage		= D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags	= D3D11_BIND_CONSTANT_BUFFER;
	hr = device->CreateBuffer(&buffer_desc, nullptr, color_cbuffer.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// インスタンス用バッファ
	buffer_desc = {};
	buffer_desc.ByteWidth			= SCast(UINT, sizeof(SbInstanceModel) * max_model_count);
	buffer_desc.Usage				= D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags			= D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	buffer_desc.CPUAccessFlags		= 0;
	buffer_desc.MiscFlags			= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	buffer_desc.StructureByteStride = sizeof(SbInstanceModel);

	hr = device->CreateBuffer(&buffer_desc, nullptr, instance_model_sbuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	
	// インスタンス用SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Format = DXGI_FORMAT_UNKNOWN;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srv_desc.Buffer.ElementOffset = 0;
	srv_desc.Buffer.NumElements = max_model_count;

	hr = device->CreateShaderResourceView(
		instance_model_sbuffer.Get(),
		&srv_desc,
		instance_srv.GetAddressOf()
	);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// インスタンス用UAV
	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
	uav_desc.Format = DXGI_FORMAT_UNKNOWN;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uav_desc.Buffer.FirstElement = 0;
	uav_desc.Buffer.Flags = 0;
	uav_desc.Buffer.NumElements = max_model_count;

	hr = device->CreateUnorderedAccessView(
		instance_model_sbuffer.Get(),
		&uav_desc,
		instance_uav.GetAddressOf()
	);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	device_context->CSSetUnorderedAccessViews(0, 1, instance_uav.GetAddressOf(), nullptr);
	device_context->CSSetShader(initializer_compute_shader.Get(), nullptr, 0);

	UINT num_threads = Align(max_model_count, 512);
	device_context->Dispatch(num_threads, 1, 1);

	ID3D11UnorderedAccessView* null_uav = nullptr;
	device_context->CSSetUnorderedAccessViews(0, 1, &null_uav, nullptr);
}

// デストラクタ
GameModel::~GameModel()
{
	images.clear();
	textures.clear();
	scenes.clear();
	nodes.clear();
	meshes.clear();
	materials.clear();

	for (auto& srv_ptr : texture_resource_views)
	{
		srv_ptr->Release();
	}
	texture_resource_views.clear();

	vertex_shader.Reset();
	pixel_shader.Reset();
	input_layout.Reset();
	primitive_cbuffer.Reset();
	color_cbuffer.Reset();
	material_resource_view.Reset();
}

// ノード読み込み
void GameModel::FetchNodes(const tinygltf::Model& gltf_model)
{
	// ノードでのfor each
	for (decltype(gltf_model.nodes)::const_reference gltf_node : gltf_model.nodes)
	{
		Node& node = nodes.emplace_back();
		node.name		= gltf_node.name;
		node.skin		= gltf_node.skin;
		node.children	= gltf_node.children;

		// ノード内の行列が存在しているとき、gltf_nodeのmatrixから読み込み
		if (!gltf_node.matrix.empty())
		{
			DirectX::XMFLOAT4X4 matrix;

			for (size_t row = 0; row < 4; row++)
				for (size_t column = 0; column < 4; column++)
					matrix(row, column) = SCast(float, gltf_node.matrix.at(4 * row + column));

			DirectX::XMVECTOR v_scaling, v_translation, v_rotation;
			bool succeed = DirectX::XMMatrixDecompose(&v_scaling, &v_rotation, &v_translation, DirectX::XMLoadFloat4x4(&matrix));
			_ASSERT_EXPR(succeed, L"Failed to decompose matrix.");

			DirectX::XMStoreFloat3(&node.scale,			v_scaling);
			DirectX::XMStoreFloat4(&node.rotation,		v_rotation);
			DirectX::XMStoreFloat3(&node.translation,	v_translation);
		}

		// ノード内の行列が存在していないとき、gltf_nodeのscale、translation、rotationから読み込み
		else
		{
			if (gltf_node.scale.size() > 0)
			{
				node.scale.x = SCast(float, gltf_node.scale.at(0));
				node.scale.y = SCast(float, gltf_node.scale.at(1));
				node.scale.z = SCast(float, gltf_node.scale.at(2));
			}

			if (gltf_node.translation.size() > 0)
			{
				node.translation.x = SCast(float, gltf_node.translation.at(0));
				node.translation.y = SCast(float, gltf_node.translation.at(1));
				node.translation.z = SCast(float, gltf_node.translation.at(2));
			}

			if (gltf_node.rotation.size() > 0)
			{
				node.rotation.x = SCast(float, gltf_node.rotation.at(0));
				node.rotation.y = SCast(float, gltf_node.rotation.at(1));
				node.rotation.z = SCast(float, gltf_node.rotation.at(2));
				node.rotation.w = SCast(float, gltf_node.rotation.at(3));
			}
		}
	}
	CumulateTransforms(nodes);
}

// ノードの計算
void GameModel::CumulateTransforms(std::vector<Node>& using_nodes)
{
	std::stack<DirectX::XMFLOAT4X4>		parent_global_transforms;
	std::function<void(int)> traverse =
		[&](int node_index)->void
		{
			Node& node = using_nodes.at(node_index);

			// scale、rotation、translation
			DirectX::XMMATRIX m_scaling		= DirectX::XMMatrixScaling(node.scale.x, node.scale.y, node.scale.z);
			DirectX::XMMATRIX m_rotation
				= DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(node.rotation.x, node.rotation.y, node.rotation.z, node.rotation.w));
			DirectX::XMMATRIX m_translation = DirectX::XMMatrixTranslation(node.translation.x, node.translation.y, node.translation.z);

			DirectX::XMStoreFloat4x4(&node.global_transform,
				m_scaling * m_rotation * m_translation * DirectX::XMLoadFloat4x4(&parent_global_transforms.top()));

			// 子ノードへ
			for (int child_index : node.children)
			{
				parent_global_transforms.push(node.global_transform);
				traverse(child_index);
				parent_global_transforms.pop();
			}
		};

	for (std::vector<int>::value_type node_index : scenes.at(0).nodes)
	{
		parent_global_transforms.push(
			{
				1,0,0,0,
				0,1,0,0,
				0,0,1,0,
				0,0,0,1,
			}
		);

		traverse(node_index);
		parent_global_transforms.pop();
	}
}

// メッシュ読み込み
void GameModel::FetchMeshes(const tinygltf::Model& gltf_model)
{
	Graphics*		graphics	= Graphics::GetInstance();
	ID3D11Device*	device		= graphics->GetDevice().Get();

	HRESULT	hr = S_OK;

	// メッシュでのfor each
	for (decltype(gltf_model.meshes)::const_reference gltf_mesh : gltf_model.meshes)
	{
		Mesh& mesh	= meshes.emplace_back();
		mesh.name	= gltf_mesh.name;

		// primitiveでのfor each
		for (decltype(gltf_mesh.primitives)::const_reference gltf_primitive : gltf_mesh.primitives)
		{
			Mesh::Primitive& primitive	= mesh.primitives.emplace_back();
			primitive.material			= gltf_primitive.material;

			// インデックスバッファ作成
			const tinygltf::Accessor&	gltf_accessor_prm		= gltf_model.accessors.at(gltf_primitive.indices);
			const tinygltf::BufferView& gltf_buffer_view_prm	= gltf_model.bufferViews.at(gltf_accessor_prm.bufferView);

			primitive.index_buffer_view = MakeBufferView(gltf_accessor_prm);

			D3D11_BUFFER_DESC buffer_desc = {};
			buffer_desc.ByteWidth	= SCast(UINT,primitive.index_buffer_view.size_in_bytes);
			buffer_desc.Usage		= D3D11_USAGE_DEFAULT;
			buffer_desc.BindFlags	= D3D11_BIND_INDEX_BUFFER;

			D3D11_SUBRESOURCE_DATA subresource_data = {};
			subresource_data.pSysMem = gltf_model.buffers.at(gltf_buffer_view_prm.buffer).data.data()
				+ gltf_buffer_view_prm.byteOffset + gltf_accessor_prm.byteOffset;

			hr = device->CreateBuffer(&buffer_desc, &subresource_data,
				primitive.index_buffer_view.buffer.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			// 頂点バッファ作成
			for (std::map<std::string, int>::const_reference gltf_attribute : gltf_primitive.attributes)
			{
				const tinygltf::Accessor&	gltf_accessor_attr		= gltf_model.accessors.at(gltf_attribute.second);
				const tinygltf::BufferView& gltf_buffer_view_attr	= gltf_model.bufferViews.at(gltf_accessor_attr.bufferView);

				BufferView vertex_buffer_view = MakeBufferView(gltf_accessor_attr);

				buffer_desc = {};
				buffer_desc.ByteWidth	= SCast(UINT, vertex_buffer_view.size_in_bytes);
				buffer_desc.Usage		= D3D11_USAGE_DEFAULT;
				buffer_desc.BindFlags	= D3D11_BIND_VERTEX_BUFFER;

				subresource_data = {};
				subresource_data.pSysMem = gltf_model.buffers.at(gltf_buffer_view_attr.buffer).data.data()
					+ gltf_buffer_view_attr.byteOffset + gltf_accessor_attr.byteOffset;

				hr = device->CreateBuffer(&buffer_desc, &subresource_data,
					vertex_buffer_view.buffer.ReleaseAndGetAddressOf());
				_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

				primitive.vertex_buffer_views.emplace(make_pair(gltf_attribute.first, vertex_buffer_view));
			}

			// 頂点バッファが存在しないときに以下のアトリビュートを入れる
			const std::vector<VertexBufferView> attributes = {
				{ "TANGENT",		{ DXGI_FORMAT_R32G32B32A32_FLOAT }},
				{ "TEXCOORD_0",		{ DXGI_FORMAT_R32G32_FLOAT} },
				{ "JOINTS_0",		{ DXGI_FORMAT_R16G16B16A16_UINT} },
				{ "WEIGHTS_0",		{ DXGI_FORMAT_R32G32B32A32_FLOAT} },
			};

			for (const VertexBufferView attribute : attributes)
			{
				if (primitive.vertex_buffer_views.find(attribute.str) == primitive.vertex_buffer_views.end()) 
				{
					primitive.vertex_buffer_views.insert(make_pair(attribute.str, attribute.buffer_view));
				}
			}
		}
	}
}

// テクスチャ読み込み
void GameModel::FetchTextures(const tinygltf::Model& gltf_model, std::string filename)
{
	HRESULT hr = S_OK;

	// テクスチャ配列に挿入
	for (const tinygltf::Texture& gltf_texture : gltf_model.textures)
	{
		Texture& texture	= textures.emplace_back();
		texture.name		= gltf_texture.name;
		texture.source		= gltf_texture.source;
	}

	// image配列に挿入
	for (const tinygltf::Image& gltf_image : gltf_model.images)
	{
		Image& image = images.emplace_back();
		image.name			= gltf_image.name;
		image.width			= gltf_image.width;
		image.height		= gltf_image.height;
		image.component		= gltf_image.component;
		image.bits			= gltf_image.bits;
		image.pixel_type	= gltf_image.pixel_type;
		image.buffer_view	= gltf_image.bufferView;
		image.mime_type		= gltf_image.mimeType;
		image.uri			= gltf_image.uri;
		image.as_is			= gltf_image.as_is;

		// gltf_imageにbuffer viewが存在する場合、テクスチャを読み込んでtexture resource view配列に挿入歌
		if (gltf_image.bufferView > -1) 
		{
			const tinygltf::BufferView& buffer_view = gltf_model.bufferViews.at(gltf_image.bufferView);
			const tinygltf::Buffer&		buffer		= gltf_model.buffers.at(buffer_view.buffer);
			const unsigned char*		data		= buffer.data.data() + buffer_view.byteOffset;
			const std::wstring			wstr_name	= ConvertStr2WStr(image.name);

			ID3D11ShaderResourceView* texture_resource_view = {};
			hr = TextureLoader::LoadTextureFromMemory(wstr_name, data, buffer_view.byteLength, &texture_resource_view);
			if (hr == S_OK)
			{
				texture_resource_views.emplace_back(texture_resource_view);
			}
		}

		// gltfファイル中の画像ファイル名から読み込み
		else
		{
			const std::filesystem::path	path(filename);
			ID3D11ShaderResourceView*	shader_resource_view = {};
			D3D11_TEXTURE2D_DESC		texture2d_desc;

			std::wstring w_filename =
				path.parent_path().concat(L"/").wstring() +
				std::wstring(gltf_image.uri.begin(), gltf_image.uri.end());

			hr = TextureLoader::LoadTextureFromFile(w_filename.c_str(), &shader_resource_view, &texture2d_desc);
			if (hr == S_OK)
			{
				texture_resource_views.emplace_back(shader_resource_view);
			}
		}
	}

}

// マテリアル読み込み
void GameModel::FetchMaterials(const tinygltf::Model& gltf_model)
{
	Graphics*		graphics	= Graphics::GetInstance();
	ID3D11Device*	device		= graphics->GetDevice().Get();

	for (decltype(gltf_model.materials)::const_reference gltf_material : gltf_model.materials)
	{
		std::vector<Material>::reference material = materials.emplace_back();

		// 名前
		material.name = gltf_material.name;

		// エミッシブ値
		material.data.emissive_factor[0] = SCast(float, gltf_material.emissiveFactor.at(0));
		material.data.emissive_factor[1] = SCast(float, gltf_material.emissiveFactor.at(1));
		material.data.emissive_factor[2] = SCast(float, gltf_material.emissiveFactor.at(2));

		// アルファモードの設定
		material.data.alpha_mode = gltf_material.alphaMode ==
			"OPAQUE" ? 0 : gltf_material.alphaMode ==
			"MASK" ? 1 : gltf_material.alphaMode ==
			"BLEND" ? 2 : 0;

		// アルファクリップのしきい値
		material.data.alpha_cutoff = SCast(float, gltf_material.alphaCutoff);

		// マテリアルが両面にあるか片面だけか
		material.data.double_sided = gltf_material.doubleSided ? 1 : 0;

		// ベースカラー値
		material.data.pbr_metallic_roughness.basecolor_factor[0] =
			SCast(float, gltf_material.pbrMetallicRoughness.baseColorFactor.at(0));
		material.data.pbr_metallic_roughness.basecolor_factor[1] =
			SCast(float, gltf_material.pbrMetallicRoughness.baseColorFactor.at(1));
		material.data.pbr_metallic_roughness.basecolor_factor[2] =
			SCast(float, gltf_material.pbrMetallicRoughness.baseColorFactor.at(2));
		material.data.pbr_metallic_roughness.basecolor_factor[3] =
			SCast(float, gltf_material.pbrMetallicRoughness.baseColorFactor.at(3));

		// ベースカラーのテクスチャ
		material.data.pbr_metallic_roughness.basecolor_texture.index =
			gltf_material.pbrMetallicRoughness.baseColorTexture.index;
		material.data.pbr_metallic_roughness.basecolor_texture.texcoord =
			gltf_material.pbrMetallicRoughness.baseColorTexture.texCoord;

		// メタリック値
		material.data.pbr_metallic_roughness.metallic_factor =
			SCast(float, gltf_material.pbrMetallicRoughness.metallicFactor);

		// メタリックのテクスチャ
		material.data.pbr_metallic_roughness.metallic_roughness_texture.index =
			gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index;
		material.data.pbr_metallic_roughness.metallic_roughness_texture.texcoord =
			gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.texCoord;

		// 法線のテクスチャ
		material.data.normal_texture.index = gltf_material.normalTexture.index;
		material.data.normal_texture.texcoord = gltf_material.normalTexture.texCoord;
		material.data.normal_texture.scale = SCast(float, gltf_material.normalTexture.scale);

		// 遮蔽表現のテクスチャとそのパラメータ
		material.data.occlusion_texture.index = gltf_material.occlusionTexture.index;
		material.data.occlusion_texture.texcoord = gltf_material.occlusionTexture.texCoord;
		material.data.occlusion_texture.strength =
			SCast(float, gltf_material.occlusionTexture.strength);

		// エミッシブのテクスチャ
		material.data.emissive_texture.index = gltf_material.emissiveTexture.index;
		material.data.emissive_texture.texcoord = gltf_material.emissiveTexture.texCoord;

		// extensionとextra
		material.extensions = gltf_material.extensions;
		material.extras = gltf_material.extras;

		// Extensions読み込み
		for (tinygltf::ExtensionMap::const_reference extension : material.extensions)
		{
			// クリアコート
			if (extension.first == "KHR_materials_clearcoat")
			{
				if (extension.second.Has("clearcoatFactor"))
				{
					std::vector<tinygltf::Value>::const_reference clearcoat_factor = extension.second.Get("clearcoatFactor");
					material.data.khr_materials_clearcoat.clearcoat_factor = SCast(float, clearcoat_factor.GetNumberAsDouble());
				}
				if (extension.second.Has("clearcoatTexture"))
				{
					std::vector<tinygltf::Value>::const_reference clearcoat_texture = extension.second.Get("clearcoatTexture");
					material.data.khr_materials_clearcoat.clearcoat_texture.Init(clearcoat_texture.Get<tinygltf::Value::Object>());
				}
				if (extension.second.Has("clearcoatRoughnessFactor"))
				{
					std::vector<tinygltf::Value>::const_reference clearcoat_roughness_factor = extension.second.Get("clearcoatRoughnessFactor");
					material.data.khr_materials_clearcoat.clearcoat_roughness_factor = SCast(float, clearcoat_roughness_factor.GetNumberAsDouble());
				}
				if (extension.second.Has("clearcoatRoughnessTexture"))
				{
					std::vector<tinygltf::Value>::const_reference clearcoat_roughness_texture = extension.second.Get("clearcoatRoughnessTexture");
					material.data.khr_materials_clearcoat.clearcoat_roughness_texture.Init(clearcoat_roughness_texture.Get<tinygltf::Value::Object>());
				}
				if (extension.second.Has("clearcoatNormalTexture"))
				{
					std::vector<tinygltf::Value>::const_reference clearcoat_normal_texture = extension.second.Get("clearcoatNormalTexture");
					material.data.khr_materials_clearcoat.clearcoat_normal_texture.Init(clearcoat_normal_texture.Get<tinygltf::Value::Object>());
				}
			}

			// 伝播
			if (extension.first == "KHR_materials_transmission")
			{
				if (extension.second.Has("transmissionFactor"))
				{
					std::vector<tinygltf::Value>::const_reference transmission_factor = extension.second.Get("transmissionFactor");
					material.data.khr_materials_transmission.transmission_factor = SCast(float, transmission_factor.GetNumberAsDouble());
				}
				if (extension.second.Has("transmissionTexture"))
				{
					std::vector<tinygltf::Value>::const_reference transmission_texture = extension.second.Get("transmissionTexture");
					material.data.khr_materials_transmission.transmission_texture.Init(transmission_texture.Get<tinygltf::Value::Object>());
				}
			}

			// 放射の強度
			if (extension.first == "KHR_materials_emissive_strength")
			{
				if (extension.second.Has("emissiveStrength"))
				{
					std::vector<tinygltf::Value>::const_reference emissive_strength = extension.second.Get("emissiveStrength");
					material.data.khr_materials_emissive_strength.emissive_strength = SCast(float, emissive_strength.GetNumberAsDouble());
				}
			}

			// スペキュラー
			if (extension.first == "KHR_materials_specular")
			{
				if (extension.second.Has("specularFactor"))
				{
					std::vector<tinygltf::Value>::const_reference specular_factor = extension.second.Get("specularFactor");
					material.data.khr_materials_specular.specular_factor = SCast(float, specular_factor.GetNumberAsDouble());
				}
				if (extension.second.Has("specularTexture"))
				{
					std::vector<tinygltf::Value>::const_reference specular_texture = extension.second.Get("specularTexture");
					material.data.khr_materials_specular.specular_texture.Init(specular_texture.Get<tinygltf::Value::Object>());
				}
				if (extension.second.Has("specularColorFactor"))
				{
					std::vector<tinygltf::Value>::const_reference specular_color_factor = extension.second.Get("specularColorFactor");
					material.data.khr_materials_specular.specular_color_factor.x = SCast(float, specular_color_factor.Get<tinygltf::Value::Array>().at(0).Get<double>());
					material.data.khr_materials_specular.specular_color_factor.y = SCast(float, specular_color_factor.Get<tinygltf::Value::Array>().at(1).Get<double>());
					material.data.khr_materials_specular.specular_color_factor.z = SCast(float, specular_color_factor.Get<tinygltf::Value::Array>().at(2).Get<double>());
				}
				if (extension.second.Has("specularColorTexture"))
				{
					std::vector<tinygltf::Value>::const_reference specular_color_texture = extension.second.Get("specularColorTexture");
					material.data.khr_materials_specular.specular_color_texture.Init(specular_color_texture.Get<tinygltf::Value::Object>());
				}
			}

			// ior
			if (extension.first == "KHR_materials_ior")
			{
				if (extension.second.Has("ior"))
				{
					std::vector<tinygltf::Value>::const_reference ior = extension.second.Get("ior");
					material.data.khr_materials_ior.ior = SCast(float, ior.GetNumberAsDouble());
				}
			}
		}
	}

	// ダミーマテリアル挿入
	if (materials.size() <= 0)
	{
		Material material;
		materials.emplace_back(material);
	}


	// GPUにSRVとしてマテリアルデータを保存
	std::vector<Material::SbMaterial> material_data;
	for (std::vector<Material>::const_reference material : materials)
	{
		material_data.emplace_back(material.data);
	}

	// マテリアル(SRV)を作成
	HRESULT hr;
	ComPtr<ID3D11Buffer> material_buffer;
	D3D11_BUFFER_DESC buffer_desc = {};
	buffer_desc.ByteWidth = SCast(UINT, sizeof(Material::SbMaterial) * material_data.size());
	buffer_desc.StructureByteStride = sizeof(Material::SbMaterial);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	D3D11_SUBRESOURCE_DATA subresource_data = {};
	subresource_data.pSysMem = material_data.data();
	hr = device->CreateBuffer(&buffer_desc, &subresource_data, material_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
	shader_resource_view_desc.Format = DXGI_FORMAT_UNKNOWN;
	shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	shader_resource_view_desc.Buffer.NumElements = SCast(UINT, material_data.size());
	hr = device->CreateShaderResourceView(material_buffer.Get(),
		&shader_resource_view_desc, material_resource_view.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void GameModel::InstanceUpdate(ID3D11ComputeShader* replace_cs)
{
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	device_context->CSSetUnorderedAccessViews(0, 1, instance_uav.GetAddressOf(), nullptr);
	device_context->CSSetConstantBuffers(SCast(UINT, EnumBufferglTFModel::PRIMITIVE), 1, primitive_cbuffer.GetAddressOf());
	device_context->CSSetConstantBuffers(SCast(UINT, EnumBufferglTFModel::OBJECT_COLOR), 1, color_cbuffer.GetAddressOf());

	device_context->CSSetShader(replace_cs ? replace_cs : compute_shader.Get(), NULL, 0);

	UINT num_threads = Align(max_model_count, 512);

	device_context->Dispatch(num_threads / 512, 1, 1);

	ID3D11UnorderedAccessView* null_uav = nullptr;
	device_context->CSSetUnorderedAccessViews(0, 1, &null_uav, NULL);
}

// 描画処理(色指定なし)
void GameModel::Render(bool using_shadow_map, const DirectX::XMFLOAT4X4& transform, ID3D11PixelShader* replaced_ps)
{
	Render(using_shadow_map, transform, { 1.0f,1.0f,1.0f }, replaced_ps);
}

// 描画処理(色指定あり)
void GameModel::Render(bool using_shadow_map, const DirectX::XMFLOAT4X4& transform, DirectX::XMFLOAT3 color, ID3D11PixelShader* replaced_ps)
{
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	device_context->PSSetShaderResources(0, 1, material_resource_view.GetAddressOf());

	if(mask_srv)
		device_context->PSSetShaderResources(30, 1, mask_srv.GetAddressOf());

	device_context->VSSetShader(vertex_shader.Get(), nullptr, 0);
	if (replaced_ps)
	{
		device_context->PSSetShader(replaced_ps, nullptr, 0);
	}
	else if (using_shadow_map)
	{
		device_context->PSSetShader(nullptr, nullptr, 0);
	}
	else
	{
		device_context->PSSetShader(pixel_shader.Get(), nullptr, 0);
	}
	device_context->IASetInputLayout(input_layout.Get());
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//for(Mesh& mesh : meshes)

	// シーンの個数分ループ
	for(auto& scene : scenes)
	{
		// ノードの個数分もループ
		for (int node_index : scene.nodes)
		{
			RenderTraverse(node_index, transform, color);
		}
	}
}

void GameModel::RenderTraverse(size_t node_index, const DirectX::XMFLOAT4X4 transform, DirectX::XMFLOAT3 color)
{
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	const Node& NODE = nodes.at(node_index);
	if (NODE.skin > -1)
	{
		const Skin& SKIN = skins.at(NODE.skin);
		CbPrimitiveJoint primitive_joint_constants = {};

		size_t max_joint_size = SKIN.joints.size();
		for (size_t joint_index = 0; joint_index < max_joint_size; joint_index++)
		{
			DirectX::XMStoreFloat4x4(&primitive_joint_constants.matrices[joint_index],
				DirectX::XMLoadFloat4x4(&SKIN.inverse_bind_matrices.at(joint_index))
				* DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&NODE.global_transform)));
		}

		device_context->UpdateSubresource(primitive_joint_cbuffer.Get(), 0, 0, &primitive_joint_constants, 0, 0);
		device_context->VSSetConstantBuffers(SCast(UINT, EnumBufferglTFModel::PRIMITIVE_JOINT), 1, primitive_cbuffer.GetAddressOf());
		device_context->PSSetConstantBuffers(SCast(UINT, EnumBufferglTFModel::PRIMITIVE_JOINT), 1, primitive_cbuffer.GetAddressOf());
	}

	// ノードにメッシュが設定されているとき
	CbPrimitive primitive_data = {};
	if (NODE.mesh > -1)
	{
		const Mesh& mesh = meshes.at(NODE.mesh);
		for (std::vector<Mesh::Primitive>::const_reference primitive : mesh.primitives)
		{
			ID3D11Buffer* vertex_buffers[] = {
				primitive.vertex_buffer_views.at("POSITION").buffer.Get(),
				primitive.vertex_buffer_views.at("NORMAL").buffer.Get(),
				primitive.vertex_buffer_views.at("TANGENT").buffer.Get(),
				primitive.vertex_buffer_views.at("TEXCOORD_0").buffer.Get(),
				primitive.vertex_buffer_views.at("JOINTS_0").buffer.Get(),
				primitive.vertex_buffer_views.at("WEIGHTS_0").buffer.Get(),
			};

			UINT strides[] = {
				SCast(UINT, primitive.vertex_buffer_views.at("POSITION").stride_in_bytes),
				SCast(UINT, primitive.vertex_buffer_views.at("NORMAL").stride_in_bytes),
				SCast(UINT, primitive.vertex_buffer_views.at("TANGENT").stride_in_bytes),
				SCast(UINT, primitive.vertex_buffer_views.at("TEXCOORD_0").stride_in_bytes),
				SCast(UINT, primitive.vertex_buffer_views.at("JOINTS_0").stride_in_bytes),
				SCast(UINT, primitive.vertex_buffer_views.at("WEIGHTS_0").stride_in_bytes),
			};

			UINT offsets[_countof(vertex_buffers)] = { 0 };
			device_context->IASetVertexBuffers(0, _countof(vertex_buffers), vertex_buffers, strides, offsets);
			device_context->IASetIndexBuffer(primitive.index_buffer_view.buffer.Get(),
				primitive.index_buffer_view.format, 0);

			primitive_data.material = primitive.material;
			primitive_data.has_tangent = primitive.vertex_buffer_views.at("TANGENT").buffer != nullptr;
			XMStoreFloat4x4(&primitive_data.world, XMLoadFloat4x4(&transform));

			const Material& MATERIAL = materials.at(primitive.material);
			std::vector<int> texture_indices;
			texture_indices.push_back(MATERIAL.data.pbr_metallic_roughness.basecolor_texture.index);
			texture_indices.push_back(MATERIAL.data.pbr_metallic_roughness.metallic_roughness_texture.index);
			texture_indices.push_back(MATERIAL.data.normal_texture.index);
			texture_indices.push_back(MATERIAL.data.emissive_texture.index);
			texture_indices.push_back(MATERIAL.data.occlusion_texture.index);
			texture_indices.push_back(MATERIAL.data.khr_materials_clearcoat.clearcoat_texture.index);
			texture_indices.push_back(MATERIAL.data.khr_materials_clearcoat.clearcoat_roughness_texture.index);
			texture_indices.push_back(MATERIAL.data.khr_materials_clearcoat.clearcoat_normal_texture.index);
			texture_indices.push_back(MATERIAL.data.khr_materials_transmission.transmission_texture.index);
			texture_indices.push_back(MATERIAL.data.khr_materials_specular.specular_texture.index);
			texture_indices.push_back(MATERIAL.data.khr_materials_specular.specular_color_texture.index);

			ID3D11ShaderResourceView* null_shader_resource_view = {};
			std::vector<ID3D11ShaderResourceView*>	shader_resource_views(texture_indices.size());

			for (int texture_index = 0; texture_index < texture_resource_views.size(); ++texture_index)
			{
				shader_resource_views.at(texture_index) = texture_indices.at(texture_index) > -1 ?
					texture_resource_views.at(textures.at(texture_indices.at(texture_index)).source) :
					null_shader_resource_view;
			}

			device_context->PSSetShaderResources(MATERIAL_START_INDEX, SCast(UINT, shader_resource_views.size()),
				shader_resource_views.data());

			device_context->UpdateSubresource(primitive_cbuffer.Get(), 0, 0, &primitive_data, 0, 0);
			device_context->VSSetConstantBuffers(SCast(UINT, EnumBufferglTFModel::PRIMITIVE), 1, primitive_cbuffer.GetAddressOf());
			device_context->PSSetConstantBuffers(SCast(UINT, EnumBufferglTFModel::PRIMITIVE), 1, primitive_cbuffer.GetAddressOf());

			CbObjectColor color_constant;
			color_constant.object_color = color;
			color_constant.blink_factor = blink_factor;
			device_context->UpdateSubresource(color_cbuffer.Get(), 0, nullptr, &color_constant, 0, 0);
			device_context->VSSetConstantBuffers(SCast(UINT, EnumBufferglTFModel::OBJECT_COLOR), 1, color_cbuffer.GetAddressOf());
			device_context->PSSetConstantBuffers(SCast(UINT, EnumBufferglTFModel::OBJECT_COLOR), 1, color_cbuffer.GetAddressOf());

			//device_context->DrawIndexed(SCast(UINT, primitive.index_buffer_view.Count()), 0, 0);
			device_context->DrawIndexedInstanced(SCast(UINT, primitive.index_buffer_view.Count()), drawing_model_count, 0, 0, 0);
		}
	}

	// ノードにメッシュが指定されていないとき
	else
	{
		for (const Mesh& mesh : meshes)
		{
			for (std::vector<Mesh::Primitive>::const_reference primitive : mesh.primitives)
			{
				ID3D11Buffer* vertex_buffers[] = {
					primitive.vertex_buffer_views.at("POSITION").buffer.Get(),
					primitive.vertex_buffer_views.at("NORMAL").buffer.Get(),
					primitive.vertex_buffer_views.at("TANGENT").buffer.Get(),
					primitive.vertex_buffer_views.at("TEXCOORD_0").buffer.Get(),
					primitive.vertex_buffer_views.at("JOINTS_0").buffer.Get(),
					primitive.vertex_buffer_views.at("WEIGHTS_0").buffer.Get(),
				};

				UINT strides[] = {
					SCast(UINT, primitive.vertex_buffer_views.at("POSITION").stride_in_bytes),
					SCast(UINT, primitive.vertex_buffer_views.at("NORMAL").stride_in_bytes),
					SCast(UINT, primitive.vertex_buffer_views.at("TANGENT").stride_in_bytes),
					SCast(UINT, primitive.vertex_buffer_views.at("TEXCOORD_0").stride_in_bytes),
					SCast(UINT, primitive.vertex_buffer_views.at("JOINTS_0").stride_in_bytes),
					SCast(UINT, primitive.vertex_buffer_views.at("WEIGHTS_0").stride_in_bytes),
				};

				UINT offsets[_countof(vertex_buffers)] = { 0 };
				device_context->IASetVertexBuffers(0, _countof(vertex_buffers), vertex_buffers, strides, offsets);
				device_context->IASetIndexBuffer(primitive.index_buffer_view.buffer.Get(),
					primitive.index_buffer_view.format, 0);

				primitive_data = {};
				primitive_data.material = primitive.material;
				primitive_data.has_tangent = primitive.vertex_buffer_views.at("TANGENT").buffer != nullptr;
				XMStoreFloat4x4(&primitive_data.world,
					XMLoadFloat4x4(&transform));

				const Material& material = materials.at(primitive.material);
				std::vector<int> texture_indices;
				texture_indices.push_back(material.data.pbr_metallic_roughness.basecolor_texture.index);
				texture_indices.push_back(material.data.pbr_metallic_roughness.metallic_roughness_texture.index);
				texture_indices.push_back(material.data.normal_texture.index);
				texture_indices.push_back(material.data.emissive_texture.index);
				texture_indices.push_back(material.data.occlusion_texture.index);
				texture_indices.push_back(material.data.khr_materials_clearcoat.clearcoat_texture.index);
				texture_indices.push_back(material.data.khr_materials_clearcoat.clearcoat_roughness_texture.index);
				texture_indices.push_back(material.data.khr_materials_clearcoat.clearcoat_normal_texture.index);
				texture_indices.push_back(material.data.khr_materials_transmission.transmission_texture.index);
				texture_indices.push_back(material.data.khr_materials_specular.specular_texture.index);
				texture_indices.push_back(material.data.khr_materials_specular.specular_color_texture.index);

				std::vector<ID3D11ShaderResourceView*>	shader_resource_views(texture_indices.size());

				for (int texture_index = 0; texture_index < texture_indices.size(); ++texture_index)
				{
					if (texture_indices.at(texture_index) < 0)	continue;
					shader_resource_views.at(texture_index)
						= texture_resource_views.at(textures.at(texture_indices.at(texture_index)).source);
				}

				device_context->PSSetShaderResources(MATERIAL_START_INDEX, SCast(UINT, shader_resource_views.size()), shader_resource_views.data());

				device_context->UpdateSubresource(primitive_cbuffer.Get(), 0, 0, &primitive_data, 0, 0);
				device_context->VSSetConstantBuffers(SCast(UINT, EnumBufferglTFModel::PRIMITIVE), 1, primitive_cbuffer.GetAddressOf());
				device_context->PSSetConstantBuffers(SCast(UINT, EnumBufferglTFModel::PRIMITIVE), 1, primitive_cbuffer.GetAddressOf());

				CbObjectColor color_constant;
				color_constant.object_color		= color;
				color_constant.blink_factor		= blink_factor;
				color_constant.object_disolve	= disolve_factor;
				device_context->UpdateSubresource(color_cbuffer.Get(), 0, 0, &color_constant, 0, 0);
				device_context->VSSetConstantBuffers(SCast(UINT, EnumBufferglTFModel::OBJECT_COLOR), 1, color_cbuffer.GetAddressOf());
				device_context->PSSetConstantBuffers(SCast(UINT, EnumBufferglTFModel::OBJECT_COLOR), 1, color_cbuffer.GetAddressOf());

				//device_context->DrawIndexed(SCast(UINT, primitive.index_buffer_view.Count()), 0, 0 );
				device_context->DrawIndexedInstanced(SCast(UINT, primitive.index_buffer_view.Count()), drawing_model_count, 0, 0, 0);
			}
		}
	}

	// 子ノードで再帰処理
	for (int child_index : NODE.children)
	{
		RenderTraverse(child_index, primitive_data.world, color);
	}
}

// ImGui表示
void GameModel::DebugGUI()
{
	if (ImGui::Begin("gltf_model"))
	{
		if (ImGui::CollapsingHeader("scene"))
		{
			for (Scene scene : scenes) {
				ImGui::Text(scene.name.c_str());
			}
		}
		if (ImGui::CollapsingHeader("mesh"))
		{
			for (Mesh mesh : meshes) {
				ImGui::Text(mesh.name.c_str());
			}
		}
		if (ImGui::CollapsingHeader("MATERIAL"))
		{
			for (Material material : materials) {
				ImGui::Text(material.name.c_str());
			}
		}

		if (ImGui::CollapsingHeader("texture"))
		{
			for (int texture_index = 0; texture_index < texture_resource_views.size(); ++texture_index)
			{
				ID3D11ShaderResourceView* texture = texture_resource_views.at(texture_index);
				ImGui::Image(RCast(void*, texture), ImVec2(300, 300));
			}
		}
		if (ImGui::CollapsingHeader("image"))
		{
			for (Image image : images) {
				ImGui::Text(image.name.c_str());
			}
		}
	}
	ImGui::End();
}

// buffer_view作成
GameModel::BufferView GameModel::MakeBufferView(const tinygltf::Accessor& accessor)
{
	BufferView buffer_view;
	switch (accessor.type)
	{
	case TINYGLTF_TYPE_SCALAR:
		switch (accessor.componentType)
		{
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
			buffer_view.format				= DXGI_FORMAT_R16_UINT;
			buffer_view.stride_in_bytes		= sizeof(USHORT);
			break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
			buffer_view.format				= DXGI_FORMAT_R16_UINT;
			buffer_view.stride_in_bytes		= sizeof(UINT);
			break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
			buffer_view.format				= DXGI_FORMAT_R16_UINT;
			buffer_view.stride_in_bytes		= sizeof(BYTE);
			break;
		default:
			_ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
			break;
		}
		break;

	case TINYGLTF_TYPE_VEC2:
		switch (accessor.componentType)
		{
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
			buffer_view.format				= DXGI_FORMAT_R32G32_FLOAT;
			buffer_view.stride_in_bytes		= sizeof(FLOAT) * 2;
			break;
		default:
			_ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
			break;
		}
		break;

	case TINYGLTF_TYPE_VEC3:
		switch (accessor.componentType)
		{
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
			buffer_view.format				= DXGI_FORMAT_R32G32B32_FLOAT;
			buffer_view.stride_in_bytes		= sizeof(FLOAT) * 3;
			break;
		default:
			_ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
			break;
		}
		break;
	case TINYGLTF_TYPE_VEC4:
		switch (accessor.componentType)
		{
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
			buffer_view.format				= DXGI_FORMAT_R8G8B8A8_UINT;
			buffer_view.stride_in_bytes		= sizeof(BYTE) * 4;
			break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
			buffer_view.format				= DXGI_FORMAT_R16G16B16A16_UINT;
			buffer_view.stride_in_bytes		= sizeof(USHORT) * 4;
			break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
			buffer_view.format				= DXGI_FORMAT_R16G16B16A16_UINT;
			buffer_view.stride_in_bytes		= sizeof(UINT) * 4;
			break;
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
			buffer_view.format				= DXGI_FORMAT_R32G32B32A32_FLOAT;
			buffer_view.stride_in_bytes		= sizeof(FLOAT) * 4;
			break;
		default:
			_ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
			break;
		}
		break;

	default:
		_ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
		break;
	}
	buffer_view.size_in_bytes = SCast(UINT, accessor.count * buffer_view.stride_in_bytes);
	return buffer_view;
}

void GameModel::SetMaskTexture(const wchar_t* w_filename)
{
	D3D11_TEXTURE2D_DESC texture_2d_desc;
	TextureLoader::LoadTextureFromFile(w_filename, mask_srv.GetAddressOf(), &texture_2d_desc);
}
