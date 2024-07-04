#ifndef __XMFLOAT_CALCLATION_H__
#define __XMFLOAT_CALCLATION_H__

// <>インクルード
#include <DirectXMath.h>

#ifndef  XMFLOAT_CALCLATION
#define  XMFLOAT_CALCLATION

// namespace >> [XMFloatCalclation]
inline namespace XMFloatCalclation
{
	// static関数
	static DirectX::XMFLOAT4X4 XMFloat4x4Multiply(DirectX::XMFLOAT4X4 f1, DirectX::XMFLOAT4X4 f2)
	{
		DirectX::XMMATRIX m1 = DirectX::XMLoadFloat4x4(&f1);
		DirectX::XMMATRIX m2 = DirectX::XMLoadFloat4x4(&f2);

		DirectX::XMMATRIX m_result = DirectX::XMMatrixMultiply(m1, m2);

		DirectX::XMFLOAT4X4 ret;
		DirectX::XMStoreFloat4x4(&ret, m_result);

		return ret;
	}

	static DirectX::XMFLOAT4X4 XMFloat4x4MultiplyInverse(DirectX::XMFLOAT4X4 f1, DirectX::XMFLOAT4X4 f2)
	{
		DirectX::XMMATRIX m1 = DirectX::XMLoadFloat4x4(&f1);
		DirectX::XMMATRIX m2 = DirectX::XMLoadFloat4x4(&f2);

		DirectX::XMMATRIX m_result = DirectX::XMMatrixInverse(NULL, DirectX::XMMatrixMultiply(m1, m2));

		DirectX::XMFLOAT4X4 ret;
		DirectX::XMStoreFloat4x4(&ret, m_result);

		return ret;
	}

	static DirectX::XMFLOAT4 XMFloat4Add(DirectX::XMFLOAT4 f1, DirectX::XMFLOAT4 f2)
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat4(&f1);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat4(&f2);

		DirectX::XMVECTOR v_result = DirectX::XMVectorAdd(v1, v2);

		DirectX::XMFLOAT4 ret;
		DirectX::XMStoreFloat4(&ret, v_result);

		return ret;
	}

	static DirectX::XMFLOAT3 XMFloat3Add(DirectX::XMFLOAT3 f1, DirectX::XMFLOAT3 f2)
	{
		DirectX::XMFLOAT4 new_f1 = { f1.x,f1.y,f1.z,0.0f };
		DirectX::XMFLOAT4 new_f2 = { f2.x,f2.y,f2.z,0.0f };
		DirectX::XMFLOAT4 ret_f4 = XMFloat4Add(new_f1, new_f2);
		return DirectX::XMFLOAT3(ret_f4.x, ret_f4.y, ret_f4.z);
	}

	static DirectX::XMFLOAT2 XMFloat2Add(DirectX::XMFLOAT2 f1, DirectX::XMFLOAT2 f2)
	{
		DirectX::XMFLOAT4 new_f1 = { f1.x,f1.y,0.0f,0.0f };
		DirectX::XMFLOAT4 new_f2 = { f2.x,f2.y,0.0f,0.0f };
		DirectX::XMFLOAT4 ret_f4 = XMFloat4Add(new_f1, new_f2);
		return DirectX::XMFLOAT2(ret_f4.x, ret_f4.y);
	}

	static DirectX::XMFLOAT4 XMFloat4Lerp(DirectX::XMFLOAT4 f1, DirectX::XMFLOAT4 f2,float rate)
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat4(&f1);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat4(&f2);

		DirectX::XMVECTOR v_result = DirectX::XMVectorLerp(v1, v2,rate);

		DirectX::XMFLOAT4 ret;
		DirectX::XMStoreFloat4(&ret, v_result);

		return ret;
	}

	static DirectX::XMFLOAT3 XMFloat3Lerp(DirectX::XMFLOAT3 f1, DirectX::XMFLOAT3 f2, float rate)
	{
		DirectX::XMFLOAT4 new_f1 = { f1.x,f1.y,f1.z,0.0f };
		DirectX::XMFLOAT4 new_f2 = { f2.x,f2.y,f2.z,0.0f };
		DirectX::XMFLOAT4 ret_f4 = XMFloat4Lerp(new_f1, new_f2,rate);
		return DirectX::XMFLOAT3(ret_f4.x, ret_f4.y, ret_f4.z);
	}

	static DirectX::XMFLOAT2 XMFloat2Lerp(DirectX::XMFLOAT2 f1, DirectX::XMFLOAT2 f2,float rate)
	{
		DirectX::XMFLOAT4 new_f1 = { f1.x,f1.y,0.0f,0.0f };
		DirectX::XMFLOAT4 new_f2 = { f2.x,f2.y,0.0f,0.0f };
		DirectX::XMFLOAT4 ret_f4 = XMFloat4Lerp(new_f1, new_f2,rate);
		return DirectX::XMFLOAT2(ret_f4.x, ret_f4.y);
	}

	static DirectX::XMFLOAT4 XMFloat4Subtract(DirectX::XMFLOAT4 f1, DirectX::XMFLOAT4 f2)
	{
		DirectX::XMFLOAT4 minus_f2 = { -f2.x,-f2.y,-f2.z,-f2.w };
		return XMFloat4Add(f1, minus_f2);
	}

	static DirectX::XMFLOAT3 XMFloat3Subtract(DirectX::XMFLOAT3 f1, DirectX::XMFLOAT3 f2)
	{
		DirectX::XMFLOAT3 minus_f2 = { -f2.x,-f2.y,-f2.z };
		return XMFloat3Add(f1, minus_f2);
	}

	static DirectX::XMFLOAT2 XMFloat2Subtract(DirectX::XMFLOAT2 f1, DirectX::XMFLOAT2 f2)
	{
		DirectX::XMFLOAT2 minus_f2 = { -f2.x,-f2.y };
		return XMFloat2Add(f1, minus_f2);
	}

	static DirectX::XMFLOAT4 XMFloat4Multiply(DirectX::XMFLOAT4 f1, DirectX::XMFLOAT4 f2)
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat4(&f1);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat4(&f2);

		DirectX::XMVECTOR v_result = DirectX::XMVectorMultiply(v1, v2);

		DirectX::XMFLOAT4 ret;
		DirectX::XMStoreFloat4(&ret, v_result);

		return ret;
	}

	static DirectX::XMFLOAT3 XMFloat3Multiply(DirectX::XMFLOAT3 f1, DirectX::XMFLOAT3 f2)
	{
		DirectX::XMFLOAT4 new_f1 = { f1.x,f1.y,f1.z,0.0f };
		DirectX::XMFLOAT4 new_f2 = { f2.x,f2.y,f2.z,0.0f };
		DirectX::XMFLOAT4 ret_f4 = XMFloat4Multiply(new_f1, new_f2);
		return DirectX::XMFLOAT3(ret_f4.x, ret_f4.y, ret_f4.z);
	}

	static DirectX::XMFLOAT2 XMFloat2Multiply(DirectX::XMFLOAT2 f1, DirectX::XMFLOAT2 f2)
	{
		DirectX::XMFLOAT4 new_f1 = { f1.x,f1.y,0.0f,0.0f };
		DirectX::XMFLOAT4 new_f2 = { f2.x,f2.y,0.0f,0.0f };
		DirectX::XMFLOAT4 ret_f4 = XMFloat4Multiply(new_f1, new_f2);
		return DirectX::XMFLOAT2(ret_f4.x, ret_f4.y);
	}

	static DirectX::XMFLOAT4 XMFloat4Scale(DirectX::XMFLOAT4 f, float scale)
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat4(&f);
		DirectX::XMVECTOR v_result = DirectX::XMVectorScale(v, scale);
		DirectX::XMFLOAT4 ret;
		DirectX::XMStoreFloat4(&ret, v_result);
		return ret;
	}

	static DirectX::XMFLOAT3 XMFloat3Scale(DirectX::XMFLOAT3 f, float scale)
	{
		DirectX::XMFLOAT4 new_f = { f.x,f.y,f.z,0.0f };
		DirectX::XMFLOAT4 ret_f4 = XMFloat4Scale(new_f, scale);
		return DirectX::XMFLOAT3(ret_f4.x, ret_f4.y, ret_f4.z);
	}

	static DirectX::XMFLOAT2 XMFloat2Multiply(DirectX::XMFLOAT2 f, float scale)
	{
		DirectX::XMFLOAT4 new_f = { f.x,f.y,0.0f,0.0f };
		DirectX::XMFLOAT4 ret_f4 = XMFloat4Scale(new_f, scale);
		return DirectX::XMFLOAT2(ret_f4.x, ret_f4.y);
	}

	static DirectX::XMFLOAT4 XMFloat4Divide(DirectX::XMFLOAT4 f1, DirectX::XMFLOAT4 f2)
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat4(&f1);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat4(&f2);

		DirectX::XMVECTOR v_result = DirectX::XMVectorDivide(v1, v2);

		DirectX::XMFLOAT4 ret;
		DirectX::XMStoreFloat4(&ret, v_result);

		return ret;
	}

	static DirectX::XMFLOAT3 XMFloat3Divide(DirectX::XMFLOAT3 f1, DirectX::XMFLOAT3 f2)
	{
		DirectX::XMFLOAT4 new_f1 = { f1.x,f1.y,f1.z,0.0f };
		DirectX::XMFLOAT4 new_f2 = { f2.x,f2.y,f2.z,0.0f };
		DirectX::XMFLOAT4 ret_f4 = XMFloat4Divide(new_f1, new_f2);
		return DirectX::XMFLOAT3(ret_f4.x, ret_f4.y, ret_f4.z);
	}

	static DirectX::XMFLOAT2 XMFloat2Divide(DirectX::XMFLOAT2 f1, DirectX::XMFLOAT2 f2)
	{
		DirectX::XMFLOAT4 new_f1 = { f1.x,f1.y,0.0f,0.0f };
		DirectX::XMFLOAT4 new_f2 = { f2.x,f2.y,0.0f,0.0f };
		DirectX::XMFLOAT4 ret_f4 = XMFloat4Divide(new_f1, new_f2);
		return DirectX::XMFLOAT2(ret_f4.x, ret_f4.y);
	}

	static bool XMFloat4Equal(DirectX::XMFLOAT4 f1, DirectX::XMFLOAT4 f2)
	{
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat4(&f1);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat4(&f2);

		bool result = DirectX::XMVectorGetX(DirectX::XMVectorEqual(v1, v2));

		return result;
	}

	static bool XMFloat3Equal(DirectX::XMFLOAT3 f1, DirectX::XMFLOAT3 f2)
	{
		DirectX::XMFLOAT4 new_f1 = { f1.x,f1.y,f1.z,0.0f };
		DirectX::XMFLOAT4 new_f2 = { f2.x,f2.y,f2.z,0.0f };
		bool result = XMFloat4Equal(new_f1, new_f2);
		return result;
	}

	static bool XMFloat2Equal(DirectX::XMFLOAT2 f1, DirectX::XMFLOAT2 f2)
	{
		DirectX::XMFLOAT4 new_f1 = { f1.x,f1.y,0.0f,0.0f };
		DirectX::XMFLOAT4 new_f2 = { f2.x,f2.y,0.0f,0.0f };
		bool result = XMFloat4Equal(new_f1, new_f2);
		return result;
	}
};

#else
using namespace XMFloatCalclation;

#endif // ! XMFLOAT_CALCLATION


#endif // __XMFLOAT_CALCLATION_H__