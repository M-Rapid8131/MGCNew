#ifndef __FILTER_FUNCTION_HLSLI__
#define __FILTER_FUNCTION_HLSLI__

// ŠÖ”
float3 RGB2HSV(float3 rgb)
{
	float3 hsv = 0;
	
	float v_max = max(rgb.r, max(rgb.g, rgb.b));
	float v_min = min(rgb.r, min(rgb.g, rgb.b));
	float delta = v_max - v_min;
	
	hsv.z = v_max;
	
	hsv.y = (delta / v_max) * step(0, v_max);
	
	if (hsv.y > 0.0f)
	{
		if (rgb.r == v_max)
		{
			hsv.x = 60 * ((rgb.g - rgb.b) / delta);
		}
		else if (rgb.g == v_max)
		{
			hsv.x = 60 * ((rgb.b - rgb.r) / delta) + 120;
		}
		else
		{
			hsv.x = 60 * ((rgb.r - rgb.g) / delta) + 240;
		}
		
		if (hsv.x < 0)
		{
			hsv.x += 360;
		}
	}
	return hsv;
}

float3 HSV2RGB(float3 hsv)
{
	float3 rgb = 0;
	if(hsv.y == 0)
	{
		rgb.r = rgb.g = rgb.b = hsv.z;
	}
	else
	{
		float Vmax = hsv.z;
		float Vmin = Vmax - (hsv.y * Vmax);
		
		hsv.x %= 360;
		float Huei = (int) (hsv.x / 60);
		float Huef = hsv.x / 60 - Huei;
		float p = Vmax * (1.0f - hsv.y);
		float q = Vmax * (1.0f - hsv.y * Huef);
		float t = Vmax * (1.0f - hsv.y * (1 - Huef));
		
		if(Huei == 0)
		{
			rgb.r = Vmax;
			rgb.g = t;
			rgb.b = p;
		}
		else if(Huei == 1)
		{
			rgb.r = q;
			rgb.g = Vmax;
			rgb.b = p;
		}
		else if(Huei == 2)
		{
			rgb.r = p;
			rgb.g = Vmax;
			rgb.b = t;
		}
		else if(Huei == 3)
		{
			rgb.r = p;
			rgb.g = q;
			rgb.b = Vmax;
		}
		else if(Huei == 4)
		{
			rgb.r = t;
			rgb.g = p;
			rgb.b = Vmax;
		}
		else if(Huei == 5)
		{
			rgb.r = Vmax;
			rgb.g = p;
			rgb.b = q;
		}
	}
	return rgb;
}

#endif // __FILTER_FUNCTION_HLSLI__