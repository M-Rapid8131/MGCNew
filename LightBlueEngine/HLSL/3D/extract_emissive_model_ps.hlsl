// �C���N���[�h
#include "3D_utility/material_data.hlsli"
#include "3d_header.hlsli"

// �֐�
float4 ShaderMain(VS_OUT_3D pin) : SV_TARGET
{
	const SbMaterial mtl = materials[material];
	
	MaterialData material_data = GetMaterialData(mtl, pin.texcoord);
	
	// �قړ����ł���΂��̃s�N�Z���̏������I��
	if (material_data.basecolor_factor.a < 0.01f)
		discard;
	
	float3 color = material_data.emissive_factor * object_color;
	return float4(color, material_data.emissive_strength);
}