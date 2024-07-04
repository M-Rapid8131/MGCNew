// インクルード
#include "2d_header.hlsli"

// 変数
Texture2D texture_maps[4] : register(t0);

// 関数
float4 ShaderMain(VS_OUT_2D pin) : SV_TARGET
{
	// 元の画面の色
	float4 color = texture_maps[0].Sample(sampler_states[SS_ANISOTROPIC], pin.texcoord);
	
	// ここでは加重平均法を使用。
	color.rgb = smoothstep(0.5, 0.8, dot(color.rgb, float3(0.299, 0.587, 0.114))) * color.rgb;
	return color;
}