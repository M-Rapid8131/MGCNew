#ifndef __EASING_H__
#define __EASING_H__

// <>インクルード
#include <DirectXMath.h>
#include <math.h>

enum class EnumEasingType
{
	SINE,
	QUAD,
	CUBIC,
	QUART,
	QUINT,
	EXPO,
	CIRC,
	BACK,
	ELASTIC,
	BOUNCE,
};

enum class EnumEasingMode
{
	RATE,
	VALUE
};

// namespace >> [Easing]
namespace Easing
{
	static float Out(const EnumEasingType easing_type, const float value)
	{
		if (value <= 0.0f)	return 0.0f;
		if (value >= 1.0f)	return 1.0f;

		const float C1 = 1.70158f;
		const float C3 = C1 + 1.0f;
		const float C4 = (2.0f * DirectX::XM_PI) / 3.0f;

		const float N1 = 7.5625f;

		const float D1 = 2.75f;

		switch (easing_type)
		{
		case EnumEasingType::SINE:
			return sinf((value * DirectX::XM_PI) * 0.5f);

		case EnumEasingType::QUAD:
			return 1.0f - powf(1.0f - value, 2.0f);

		case EnumEasingType::CUBIC:
			return 1.0f - powf(1.0f - value, 3.0f);

		case EnumEasingType::QUART:
			return 1.0f - powf(1.0f - value, 4.0f);

		case EnumEasingType::QUINT:
			return 1.0f - powf(1.0f - value, 5.0f);

		case EnumEasingType::EXPO:
			return (fabsf(value - 1.0f) < FLT_EPSILON) ? 1.0f : 1.0f - powf(2.0f, -10.0f * value);

		case EnumEasingType::CIRC:
			return sqrtf(1.0f - powf(value - 1.0f, 2));

		case EnumEasingType::BACK:
			return 1.0f + C3 * powf(value - 1.0f, 3.0f) + C1 * powf(value - 1.0f, 2.0f);

		case EnumEasingType::ELASTIC:
			return value < FLT_EPSILON ? 0.0f :
				fabsf(value - 1.0f) < FLT_EPSILON ? 1.0f :
				powf(2.0f, -10.0f * value) * sinf((value * 10.0f - 0.75f) * C4) + 1.0f;

		case EnumEasingType::BOUNCE:
			if (value < 1.0f / D1)
				return N1 * powf(value, 2.0f);
			else if (value < 2.0f / D1)
				return N1 * powf(value - 1.5f / D1, 2.0f) + 0.75f;
			else if (value < 2.5f / D1)
				return N1 * powf(value - 2.25f / D1, 2.0f) + 0.9375f;
			else
				return N1 * powf(value - 2.625f / D1, 2.0f) + 0.984375f;
		}
	}

	static float Out(const EnumEasingType easing_type, const EnumEasingMode easing_mode, const float from, const float to, const float value)
	{
		if (value <= from)	return easing_mode == EnumEasingMode::VALUE ? from : 0.0f;
		if (value >= to)	return easing_mode == EnumEasingMode::VALUE ? to : 1.0f;

		const float duration = to - from;
		const float rate = (value - from) / duration;

		const float easing_rate = Out(easing_type, rate);

		switch (easing_mode)
		{
		case EnumEasingMode::RATE:
			return easing_rate;
		case EnumEasingMode::VALUE:
			return from + (easing_rate * duration);
		}
		return 0.0f;
	}

	static float In(const EnumEasingType easing_type, const float value)
	{
		if (value <= 0.0f)	return 0.0f;
		if (value >= 1.0f)	return 1.0f;

		const float C1 = 1.70158f;
		const float C3 = C1 + 1.0f;
		const float C4 = (2.0f * DirectX::XM_PI) / 3.0f;

		const float N1 = 7.5625f;

		const float D1 = 2.75f;

		switch (easing_type)
		{
		case EnumEasingType::SINE:
			return 1.0f - cosf((value * DirectX::XM_PI) * 0.5f);

		case EnumEasingType::QUAD:
			return powf(value, 2.0f);

		case EnumEasingType::CUBIC:
			return powf(value, 3.0f);

		case EnumEasingType::QUART:
			return powf(value, 4.0f);

		case EnumEasingType::QUINT:
			return powf(value, 5.0f);

		case EnumEasingType::EXPO:
			return (value < FLT_EPSILON) ? 0.0f : powf(2.0f, 10.0f * value - 10.0f);

		case EnumEasingType::CIRC:
			return 1.0f - sqrtf(1.0f - powf(value, 2));

		case EnumEasingType::BACK:
			return C3 * powf(value, 3.0f) - C1 * powf(value, 2.0f);

		case EnumEasingType::ELASTIC:
			return value < FLT_EPSILON ? 0.0f :
				fabsf(value - 1.0f) < FLT_EPSILON ? 1.0f :
				-powf(2.0f, 10.0f * value - 10.0f) * sinf((value * 10.0f - 10.75f) * C4);

		case EnumEasingType::BOUNCE:
			return 1.0f - Out(EnumEasingType::BOUNCE, 1.0f - value);
		}
	}

	static float In(const EnumEasingType easing_type, const EnumEasingMode easing_mode, const float from, const float to, const float value)
	{
		if (value <= from)	return easing_mode == EnumEasingMode::VALUE ? from : 0.0f;
		if (value >= to)	return easing_mode == EnumEasingMode::VALUE ? to : 1.0f;

		const float duration = to - from;
		const float rate = (value - from) / duration;

		const float easing_rate = In(easing_type, rate);

		switch (easing_mode)
		{
		case EnumEasingMode::RATE:
			return easing_rate;
		case EnumEasingMode::VALUE:
			return from + (easing_rate * duration);
		}
		return 0.0f;
	}

}

#endif // __EASING_H__