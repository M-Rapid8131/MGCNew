// ""インクルード
// LightBlueEngine
#include "graphics/graphics.h"
#include "graphics/texture.h"
#include "shader.h"
#include "fullscreen_quad.h"

// ゲームソースファイル

//-------------------------------------------
// FullscreenQuad メンバ関数
//-------------------------------------------

// コンストラクタ
FullscreenQuad::FullscreenQuad()
{
	// シェーダーオブジェクト作成
	Shader::CreateVSFromCso("shader/fullscreen_quad_vs.cso", embedded_vertex_shader.ReleaseAndGetAddressOf(),
		nullptr, nullptr, 0);
	Shader::CreatePSFromCso("shader/fullscreen_quad_ps.cso", embedded_pixel_shader.ReleaseAndGetAddressOf());
}

// 描画
void FullscreenQuad::Render(ID3D11ShaderResourceView** shader_resource_view, 
	uint32_t start_slot, uint32_t num_views, ID3D11VertexShader* replaced_vertex_shader, ID3D11PixelShader* replaced_pixel_shader)
{
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	// 頂点バッファ、プリミティブトポロジー、インプットレイアウトを一旦リセット
	device_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	device_context->IASetInputLayout(nullptr);

	// ピクセルシェーダー、頂点シェーダーをセット
	replaced_vertex_shader ? device_context->VSSetShader(replaced_vertex_shader, 0, 0) :
		device_context->VSSetShader(embedded_vertex_shader.Get(), 0, 0);
	replaced_pixel_shader ? device_context->PSSetShader(replaced_pixel_shader, 0, 0) :
		device_context->PSSetShader(embedded_pixel_shader.Get(), 0, 0);

	// シェーダーリソースをセット
	device_context->PSSetShaderResources(start_slot, num_views, shader_resource_view);

	device_context->Draw(4, 0);
}