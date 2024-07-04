// インクルード
#include "skymap_header.hlsli"

// 関数
VS_OUT_CUBOID ShaderMain(in uint vertexid : SV_VERTEXID, uint instance_id : SV_INSTANCEID)
{
	const int INSTANCE_COUNT		= 6;
	const int VERTEX_COUNT		= 4;
	
	VS_OUT_CUBOID vout;
	const float2 position[4] =
	{
		{ -1, +1 },
		{ +1, +1 },
		{ -1, -1 },
		{ +1, -1 },
	};

	const float2 texcoord[4] =
	{
		{ 0, 0 },
		{ 1, 0 },
		{ 0, 1 },
		{ 1, 1 },
	};

	const float3 vertices[INSTANCE_COUNT][VERTEX_COUNT] =
	{
		{ { +1, +1, +1 }, { +1, +1, -1 }, { +1, -1, +1 }, { +1, -1, -1 } }, // +X
		{ { -1, +1, +1 }, { -1, +1, -1 }, { -1, -1, +1 }, { -1, -1, -1 } }, // -X
		{ { -1, +1, +1 }, { +1, +1, -1 }, { -1, +1, +1 }, { +1, +1, +1 } }, // +Y
		{ { -1, -1, +1 }, { +1, -1, +1 }, { -1, -1, -1 }, { +1, -1, -1 } }, // -Y
		{ { -1, +1, +1 }, { +1, +1, +1 }, { -1, -1, +1 }, { +1, -1, +1 } }, // +Y
		{ { +1, +1, -1 }, { -1, +1, -1 }, { +1, -1, -1 }, { -1, -1, -1 } }, // -Y
	};

	vout.position		= float4(position[vertexid], 0, 1);
	vout.texcoord		= texcoord[vertexid];
	vout.instance_id		= instance_id;
	vout.bearing			= vertices[instance_id][vertexid];
	
	return vout;
}