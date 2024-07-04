cbuffer BLOOM_CONSTANT_BUFFER : register(b8)
{
	uint	downsampled_count;
	uint3	cbbloom_ipad;
	float	bloom_extraction_threshold;
	float	bloom_intensity;
	float	smooth_rate;
	float	cbbloom_fpad;
};