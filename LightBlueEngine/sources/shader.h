#ifndef __SHADER_H__
#define __SHADER_H__

// <>インクルード
#include <wrl.h>
#include <d3d11.h>
#include <string>
#include <unordered_map>

// ""インクルード
// LightBlueEngine
#include "singleton.h"

// using
using Microsoft::WRL::ComPtr;

// namespace >> [Shader]
namespace Shader
{
	// static関数
	HRESULT CreateVSFromCso(_In_ const char*, ID3D11VertexShader**,
		ID3D11InputLayout** = NULL, D3D11_INPUT_ELEMENT_DESC* = NULL, _In_ UINT = 0);

	HRESULT CreateGSFromCso(_In_ const char*, ID3D11GeometryShader**);
	HRESULT CreatePSFromCso(_In_ const char*, ID3D11PixelShader**);
	HRESULT CreateCSFromCso(_In_ const char*, ID3D11ComputeShader**);
};

#endif // __SHADER_H__