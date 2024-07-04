// インクルード
#include "2d_header.hlsli"

// 定数
static const float COLOR_GAP_FACTOR = 0.002f;

// 変数
Texture2D color_map_2d : register(t0);

// 関数
float4 ShaderMain(VS_OUT_2D pin) : SV_TARGET
{
	// 元の画面の色をそのまま描画
	return color_map_2d.Sample(sampler_states[SS_LINEAR], pin.texcoord);
}