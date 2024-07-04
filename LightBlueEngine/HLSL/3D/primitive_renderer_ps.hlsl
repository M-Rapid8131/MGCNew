// インクルード
#include "primitive_renderer.hlsli"

// 関数
float4 ShaderMain(VS_OUT_PR pin) : SV_TARGET
{
	return float4(pin.color.rgb,0.2f);
}