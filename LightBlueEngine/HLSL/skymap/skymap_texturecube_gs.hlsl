// インクルード
#include "skymap_header.hlsli"

// 定数
static const uint VERTEX_COUNT = 3;

// 関数
[maxvertexcount(3)]
void ShaderMain(
	triangle VS_OUT_CUBOID				input[3] : SV_POSITION,
	inout TriangleStream<GS_OUT_CUBOID>	output
)
{
	for (uint i = 0; i < VERTEX_COUNT; i++)
	{
		GS_OUT_CUBOID element;
		element.position						= input[i].position;
		element.texcoord							= input[i].texcoord;
		element.bearing							= input[i].bearing;
		element.sv_render_target_array_index		= input[i].instance_id;
		output.Append(element);
	}
	
	output.RestartStrip();
}