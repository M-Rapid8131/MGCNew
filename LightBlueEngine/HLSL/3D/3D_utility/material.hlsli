#ifndef __MATERIAL_HLSLI__
#define __MATERIAL_HLSLI__

// インクルード
#include "../../common.hlsli"
#include "../Extension/clearcoat.hlsli"
#include "../Extension/transform.hlsli"
#include "../Extension/transmission.hlsli"

static const int TEX_BASECOLOR				= 0;
static const int TEX_METALLIC_ROUGHNESS		= 1;
static const int TEX_NORMAL					= 2;
static const int TEX_EMISSIVE				= 3;
static const int TEX_OCCULUSION				= 4;
static const int TEX_CLEARCOAT				= 5;
static const int TEX_CLEARCOAT_ROUGHNESS	= 6;
static const int TEX_CLEARCOAT_NORMAL		= 7;
static const int TEX_TRANSMISSION			= 8;
static const int TEX_SPECULAR				= 9;
static const int TEX_SPECULAR_COLOR			= 10;

// 構造体

// struct >> [TextureInfo]
struct TextureInfo
{
	int					index;
	int					texcoord;
	
	KHRTextureTransform	khr_texture_transform;
};

// struct >> [NormalTextureInfo]
struct NormalTextureInfo
{
	int					index;
	int					texcoord;
	float				scale;
	
	KHRTextureTransform	khr_texture_transform;
};

// struct >> [OcculusionTextureInfo]
struct OcculusionTextureInfo
{
	int					index;
	int					texcoord;
	float				strength;
	
	KHRTextureTransform	khr_texture_transform;
};

// struct >> [PbrMetallicRoughness]
struct PbrMetallicRoughness
{
	float4		basecolor_factor;
	TextureInfo basecolor_texture;
	float		metallic_factor;
	float		roughness_factor;
	TextureInfo metallic_roughness_texture;
};

// struct >> [KHRMaterialsClearcoat]
struct KHRMaterialsClearcoat
{
	float				clearcoat_factor;
	TextureInfo			clearcoat_texture;
	float				clearcoat_roughness_factor;
	TextureInfo			clearcoat_roughness_texture;
	NormalTextureInfo	clearcoat_normal_texture;
};

// struct >> [KHRMaterialsTransmission]
struct KHRMaterialsTransmission
{
	float		transmission_factor;
	TextureInfo transmission_texture;
};

// struct >> [KHRMaterialsEmissiveStrength]
struct KHRMaterialsEmissiveStrength
{
	float emissive_strength;
};

// struct >> [KHRMaterialsSpecular]
struct KHRMaterialsSpecular
{
	float		specular_factor;
	TextureInfo specular_texture;
	float3		specular_color_factor;
	TextureInfo specular_color_texture;
};

// struct >> [KHRMaterialsIor]
struct KHRMaterialsIor
{
	float ior;
};

// struct >> [SbMaterial] register : t0
struct SbMaterial
{
	bool							double_sided;
	int								alpha_mode; //"OPAQUE" : 0, "MASK" : 1, "BLEND" : 2
	float							alpha_cutoff;
	float3							emissive_factor;
	PbrMetallicRoughness			pbr_metallic_roughness;
	NormalTextureInfo				normal_texture;
	OcculusionTextureInfo			occulusion_texture;
	TextureInfo						emissive_texture;
	
	// extension系
	KHRMaterialsClearcoat			clearcoat;
	KHRMaterialsTransmission		transmission;
	KHRMaterialsEmissiveStrength	emissive_strength;
	KHRMaterialsSpecular			specular;
	KHRMaterialsIor					ior;
};

// 変数
StructuredBuffer<SbMaterial> materials : register(t0);
Texture2D material_textures[11] : register(t16);

#endif // __MATERIAL_HLSL__