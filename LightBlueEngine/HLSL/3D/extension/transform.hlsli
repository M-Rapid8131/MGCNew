#ifndef __TRANSFORM_HLSLI__
#define __TRANSFORM_HLSLI__

// ç\ë¢ëÃ
struct KHRTextureTransform
{
	int texcoord;
	float rotation;
	float2 offset;
	float2 scale;
};

// ä÷êî
float2 TransformTexcoord(float2 texcoord, KHRTextureTransform transform)
{
	float2			transformed_texcoord		= texcoord;
	const float2		OFFSET					= transform.offset;
	const float		ROTATION					= transform.rotation;
	const float2		SCALE					= transform.scale;
	
	const float COSINE	= cos(ROTATION);
	const float SINE		= sin(ROTATION);
	
	const column_major float3x3 mat =
	{
		SCALE.x * COSINE,	-SCALE.y * SINE,	OFFSET.x,
		SCALE.x * SINE,		SCALE.y * COSINE,	OFFSET.x,
		0,					0,					1
	};

	transformed_texcoord = mul(float3(texcoord.x, texcoord.y, 1), mat).xy;
	
	return transformed_texcoord;
}

#endif // __TRANSFORM_HLSLI__