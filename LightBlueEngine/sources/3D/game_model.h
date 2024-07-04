#ifndef __GAME_MODEL_H__
#define __GAME_MODEL_H__

// <>インクルード
#include <map>
#include <string>
#include <vector>
#include <unordered_map>
#include <tiny_gltf.h>

// ""インクルード
// LightBlueEngine
#include "graphics/graphics.h"
#include "engine_utility.h"

// template using
template<typename T>
using UMapArray = std::unordered_map<int, std::vector<T>>;

// enum class
enum class EnumBufferglTFModel
{
	PRIMITIVE,
	PRIMITIVE_JOINT,
	OBJECT_COLOR,
};

enum class EnumBlockColor
{
	RED,
	BLUE,
	GREEN,
	YELLOW,
	PURPLE,
	GRAY,
	UNDEFINE
};

// class >> [GameModel]
// glTFを用いた3Dモデルのクラス。
class GameModel
{
private:
	// private:定数
	static const int  INSTANCE_COUNT		= 1;
	static const UINT DEFAULT_MODEL_COUNT	= 1;
	static const UINT MATERIAL_START_INDEX	= 16;
	static const UINT PRIMITIVE_MAX_JOINTS	= 512;

	// private:構造体
	// struct >> GameModel >> [SbModel]
	struct SbInstanceModel
	{
		UINT				model_index;
		DirectX::XMFLOAT3	position;
		DirectX::XMFLOAT4	color;
	};

	// struct >> GameModel >> [Scene]
	struct Scene
	{
		// 変数
		std::string			name;

		std::vector<int>	nodes; //'root'ノードの配列
	};

	// struct >> GameModel >> [Node]
	struct Node
	{
		// 変数
		int					skin = -1;
		int					mesh = -1;
		std::string			name;

		std::vector<int>	children;

		// ローカル座標
		DirectX::XMFLOAT4	rotation			= { 0,0,0,1 };
		DirectX::XMFLOAT3	scale				= { 1,1,1 };
		DirectX::XMFLOAT3	translation			= { 0,0,0 };
		DirectX::XMFLOAT4X4 global_transform	= {
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1
		};
	};

	// struct >> GameModel >> [BufferView]
	struct BufferView
	{
		// 関数
		size_t Count() const
		{
			if (stride_in_bytes != 0)
				return size_in_bytes / stride_in_bytes;

			else
				return 0;
		}

		// 変数
		DXGI_FORMAT				format			= DXGI_FORMAT_UNKNOWN;
		ComPtr<ID3D11Buffer>	buffer;
		size_t					stride_in_bytes = 0;
		size_t					size_in_bytes	= 0;
	};

	// struct >> GameModel >> [VertexBufferView]
	struct VertexBufferView
	{
		// 変数
		std::string	str;
		BufferView	buffer_view = {};
	};


	// struct >> GameModel >> [Mesh]
	struct Mesh
	{
		// struct >> GameModel >> Mesh >> [Primitive]
		struct Primitive
		{
			int									material				= -1;
			std::map<std::string, BufferView>	vertex_buffer_views;
			BufferView							index_buffer_view	= {};
			
		};

		// 変数
		std::vector<Primitive>		primitives;
		std::string					name;

	};


	// struct >> GameModel >> [Texture]
	struct Texture
	{
		// 変数
		int			source = -1;
		std::string	name;
	};

	// struct >> GameModel >> [Image]
	struct Image
	{
		// 変数
		bool		as_is		= false;
		int			width		= -1;
		int			height		= -1;
		int			component	= -1;
		int			bits		= -1;
		int			pixel_type	= -1;
		int			buffer_view = -1;
		std::string	mime_type;
		std::string	uri;
		std::string	name;
	};

	// struct >> GameModel >> [Skin]
	struct Skin
	{
		std::vector<DirectX::XMFLOAT4X4>		inverse_bind_matrices;
		std::vector<int>						joints;
	};

	// struct >> GameModel >> [KHRTextureTransform]
	struct KHRTextureTransform
	{
		// 関数
		void Init(const tinygltf::ExtensionMap& extensions)
		{
			tinygltf::ExtensionMap::const_iterator extension = extensions.find("KHR_texture_transform");
			if (extension != extensions.end())
			{
				if (extension->second.Has("offset"))
				{
					std::vector<tinygltf::Value>::const_reference value = extension->second.Get("offset");
					offset[0] = SCast(float, value.Get<tinygltf::Value::Array>().at(0).Get<double>());
					offset[1] = SCast(float, value.Get<tinygltf::Value::Array>().at(1).Get<double>());
				}
				if (extension->second.Has("roatation"))
				{
					std::vector<tinygltf::Value>::const_reference value = extension->second.Get("rotation");
					rotation = SCast(float, value.GetNumberAsDouble());
				}
				if (extension->second.Has("scale"))
				{
					std::vector<tinygltf::Value>::const_reference value = extension->second.Get("scale");
					scale[0] = SCast(float, value.Get<tinygltf::Value::Array>().at(0).Get<double>());
					scale[1] = SCast(float, value.Get<tinygltf::Value::Array>().at(1).Get<double>());
				}
				if (extension->second.Has("texCoord"))
				{
					std::vector<tinygltf::Value>::const_reference value = extension->second.Get("texCoord");
					texcoord = value.GetNumberAsInt();
				}
			}
		}

		// 変数
		int		texcoord	= 0;
		float	rotation	= 0;
		float	offset[2]	= { 0,0 };
		float	scale[2]	= { 1,1 };
	};

	// struct >> GameModel >> [TextureInfo]
	struct TextureInfo
	{
		// 関数
		void Init(const tinygltf::Value::Object& object)
		{
			for (tinygltf::Value::Object::const_reference value : object)
			{
				if (value.first == "index")
				{
					index = value.second.GetNumberAsInt();
				}
				else if (value.first == "texCoord")
				{
					texcoord = value.second.GetNumberAsInt();
				}
				else if (value.first == "extensions")
				{
					hkr_texture_transform.Init(value.second.Get<tinygltf::Value::Object>());
				}
			}
		}

		// 変数
		int					index		= -1;
		int					texcoord		= 0;

		KHRTextureTransform	hkr_texture_transform;
	};

	// struct >> GameModel >> [NormalTextureInfo]
	struct NormalTextureInfo
	{
		// 関数
		void Init(const tinygltf::Value::Object& object)
		{
			for (tinygltf::Value::Object::const_reference value : object)
			{
				if (value.first == "index")
				{
					index = value.second.GetNumberAsInt();
				}
				else if (value.first == "texCoord")
				{
					texcoord = value.second.GetNumberAsInt();
				}
				else if (value.first == "scale")
				{
					scale = SCast(float, value.second.GetNumberAsDouble());
				}
				else if (value.first == "extensions")
				{
					hkr_texture_transform.Init(value.second.Get<tinygltf::Value::Object>());
				}
			}
		}

		// 変数
		int					index		= -1;
		int					texcoord		= 0;
		float				scale		= 1.0f;

		KHRTextureTransform	hkr_texture_transform;
	};

	// struct >> GameModel >> [OcclussionTextureInfo]
	struct OcclusionTextureInfo
	{
		// 変数
		int					index = -1;
		int					texcoord = 0;
		float				strength = 1;

		KHRTextureTransform	hkr_texture_transform;
	};

	// struct >> GameModel >> [PbrMetallicRoughness]
	struct PBRMetallicRoughness
	{
		// 変数
		float		basecolor_factor[4] = { 1,1,1,1 };
		TextureInfo basecolor_texture;
		float		metallic_factor		= 1;
		float		roughness_factor		= 1;
		TextureInfo metallic_roughness_texture;
	};

	// struct >> GameModel >> [KHRMaterialsClearcoat]
	struct KHRMaterialsClearcoat
	{
		float				clearcoat_factor = 0.0f;
		TextureInfo			clearcoat_texture;
		float				clearcoat_roughness_factor = 0.0f;
		TextureInfo			clearcoat_roughness_texture;
		NormalTextureInfo	clearcoat_normal_texture;
	};

	// struct >> GameModel >> [KHRMaterialsTransmission]
	struct KHRMaterialsTransmission
	{
		float		transmission_factor = 0.0f;
		TextureInfo transmission_texture;
	};

	// struct >> GameModel >> [KHRMaterialsEmissiveStrength]
	struct KHRMaterialsEmissiveStrength
	{
		float emissive_strength = 1.0f;
	};

	// struct >> GameModel >> [KHRMaterialsSpecular]
	struct KHRMaterialsSpecular
	{
		float				specular_factor = 1.0f;
		TextureInfo			specular_texture;
		DirectX::XMFLOAT3	specular_color_factor = { 1.0f,1.0f,1.0f };
		TextureInfo			specular_color_texture;
	};

	// struct >> GameModel >> [KHRMaterialsIor]
	struct KHRMaterialsIor
	{
		float ior = 1.5f;
	};

	// struct >> GameModel >> [Material]
	struct Material
	{
		// struct >> GameModel >> [SbMaterial] register : t0
		struct SbMaterial
		{
			// 変数
			bool							double_sided = false;
			int								alpha_mode = 0; //"OPAQUE" : 0, "MASK" : 1, "BLEND" : 2
			float							alpha_cutoff = 0.5f;
			float							emissive_factor[3] = { 0,0,0 };
			PBRMetallicRoughness			pbr_metallic_roughness;
			NormalTextureInfo				normal_texture;
			OcclusionTextureInfo			occlusion_texture;
			TextureInfo						emissive_texture;

			// extension系
			KHRMaterialsClearcoat			khr_materials_clearcoat;
			KHRMaterialsTransmission		khr_materials_transmission;
			KHRMaterialsEmissiveStrength	khr_materials_emissive_strength;
			KHRMaterialsSpecular			khr_materials_specular;
			KHRMaterialsIor					khr_materials_ior;
		};

		std::string				name;
		SbMaterial				data;
		tinygltf::ExtensionMap	extensions;
		tinygltf::Value			extras;
	};

	// private:定数バッファ構造体

	// struct >> GameModel >> [CbPrimitive] register : b0
	struct CbPrimitive
	{
		DirectX::XMFLOAT4X4	world;
		int					has_tangent = 0;
		int					material = -1;
		int					skin = -1;

		int					cbprimitive_ipad;
	};

	// struct >> GameModel >> [CbPrimitiveJoint] register : b1
	struct CbPrimitiveJoint
	{
		DirectX::XMFLOAT4X4 matrices[PRIMITIVE_MAX_JOINTS];
	};

protected:
	// protected:定数バッファ構造体
	// struct >> GameModel >> [CbObjectColor] register : b2
	struct CbObjectColor
	{
		DirectX::XMFLOAT3	object_color = {1.0f, 1.0f, 1.0f};
		float				blink_factor = 0.0f;
		float				object_disolve = 1.0f;
		float				cbobject_color_fpad[3];
	};

public:
	// public:コンストラクタ・デストラクタ
	GameModel(const std::string&);
	virtual ~GameModel();

	// public:通常関数
	void			FetchNodes(const tinygltf::Model&);
	void			CumulateTransforms(std::vector<Node>&);
	void			FetchMeshes(const tinygltf::Model&);
	void			FetchTextures(const tinygltf::Model&, std::string);
	void			FetchMaterials(const tinygltf::Model&);
	void			InstanceUpdate(ID3D11ComputeShader* = nullptr);
	void			Render(bool, const DirectX::XMFLOAT4X4&, ID3D11PixelShader* = nullptr);
	void			Render(bool, const DirectX::XMFLOAT4X4&, DirectX::XMFLOAT3, ID3D11PixelShader* = nullptr);
	void			RenderTraverse(size_t, const  DirectX::XMFLOAT4X4, DirectX::XMFLOAT3);
	void			DebugGUI();
	BufferView		MakeBufferView(const tinygltf::Accessor&);

	// public:ゲッター関数
	float& GetDisolveFactor() { return disolve_factor; }

	// public:セッター関数
	void SetBlinkFactor(const float blink) { blink_factor = blink; }
	void SetMaskTexture(const wchar_t* w_filename);

private:
	// private:変数
	float											blink_factor = 0.0f;
	float											disolve_factor = 1.0f;
	UINT											max_model_count = DEFAULT_MODEL_COUNT;
	UINT											drawing_model_count = 1;
	ComPtr<ID3D11VertexShader>						vertex_shader;
	ComPtr<ID3D11PixelShader>						pixel_shader;
	ComPtr<ID3D11ComputeShader>						compute_shader;
	ComPtr<ID3D11ComputeShader>						initializer_compute_shader;
	ComPtr<ID3D11InputLayout>						input_layout;
	ComPtr<ID3D11Buffer>							primitive_cbuffer;
	ComPtr<ID3D11Buffer>							primitive_joint_cbuffer;
	ComPtr<ID3D11Buffer>							color_cbuffer;
	ComPtr<ID3D11Buffer>							instance_model_sbuffer;
	ComPtr<ID3D11ShaderResourceView>				material_resource_view;
	ComPtr<ID3D11ShaderResourceView>				instance_srv;
	ComPtr<ID3D11ShaderResourceView>				mask_srv;
	ComPtr<ID3D11UnorderedAccessView>				instance_uav;

	std::unique_ptr<SbInstanceModel[]>				instance_model;
	std::vector<Scene>								scenes;
	std::vector<Node>								nodes;
	std::vector<Mesh>								meshes;
	std::vector<Texture>							textures;
	std::vector<Image>								images;
	std::vector<Skin>								skins;
	std::vector<Material>							materials;
	std::vector<ID3D11ShaderResourceView*>			texture_resource_views;
	std::map<std::string, std::vector<int>>			material_texture_indices;

	std::string		gltf_text;
};

#endif // __GAME_MODEL_H__