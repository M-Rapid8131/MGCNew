// <>インクルード
#include <sstream>
#include <vector>

// ""インクルード
// LightBlueEngine
#include "shader.h"
#include "graphics/graphics.h"

// ゲームソースファイル
#include "misc.h"

// 関数
std::unique_ptr<unsigned char[]> ReadCSOFileAndGetCSOData(const char* cso_name, long& cso_sz)
{
	FILE* fp = nullptr;
	fopen_s(&fp, cso_name, "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	cso_sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data = std::make_unique<unsigned char[]>(cso_sz);

	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	return std::move(cso_data);
}

//--------------------------------------------
// ShaderPipeline メンバ関数(static)
//--------------------------------------------

// 頂点シェーダー作成
HRESULT Shader::CreateVSFromCso(_In_ const char* cso_name, ID3D11VertexShader** vertex_shader,
	ID3D11InputLayout** input_layout, D3D11_INPUT_ELEMENT_DESC* input_element_desc, _In_ UINT num_elements)
{
	Graphics*		graphics	= Graphics::GetInstance();
	ID3D11Device*	device		= graphics->GetDevice().Get();

	long								cso_sz;
	std::unique_ptr<unsigned char[]>	cso_data = ReadCSOFileAndGetCSOData(cso_name, cso_sz);;

	HRESULT hr = S_OK;
	hr = device->CreateVertexShader(cso_data.get(), cso_sz, nullptr, vertex_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	if (input_layout)
	{
		hr = device->CreateInputLayout(input_element_desc, num_elements,
			cso_data.get(), cso_sz, input_layout);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	return hr;
}

// ピクセルシェーダー作成
HRESULT Shader::CreatePSFromCso(_In_ const char* cso_name, ID3D11PixelShader** pixel_shader)
{
	Graphics*		graphics	= Graphics::GetInstance();
	ID3D11Device*	device		= graphics->GetDevice().Get();

	long								cso_sz;
	std::unique_ptr<unsigned char[]>	cso_data = ReadCSOFileAndGetCSOData(cso_name, cso_sz);;

	HRESULT hr = S_OK;
	hr = device->CreatePixelShader(cso_data.get(), cso_sz, nullptr, pixel_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	return hr;
}

// ジオメトリシェーダー作成
HRESULT Shader::CreateGSFromCso(_In_ const char* cso_name, ID3D11GeometryShader** geometry_shader)
{
	Graphics*		graphics	= Graphics::GetInstance();
	ID3D11Device*	device		= graphics->GetDevice().Get();

	long								cso_sz;
	std::unique_ptr<unsigned char[]>	cso_data = ReadCSOFileAndGetCSOData(cso_name, cso_sz);;

	HRESULT hr = S_OK;
	hr = device->CreateGeometryShader(cso_data.get(), cso_sz, nullptr, geometry_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	return hr;
}

// コンピュートシェーダー作成
HRESULT Shader::CreateCSFromCso(_In_ const char* cso_name, ID3D11ComputeShader** compute_shader)
{
	Graphics*		graphics	= Graphics::GetInstance();
	ID3D11Device*	device		= graphics->GetDevice().Get();

	long								cso_sz;
	std::unique_ptr<unsigned char[]>	cso_data = ReadCSOFileAndGetCSOData(cso_name, cso_sz);;

	HRESULT hr = S_OK;
	hr = device->CreateComputeShader(cso_data.get(), cso_sz, nullptr, compute_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	return hr;
}