// ""インクルード
// LightBlueEngine
#include "graphics/graphics.h"
#include "graphics/texture.h"
#include "engine_utility.h"
#include "shader.h"
#include "sprite.h"

// ゲームソースファイル
#include "misc.h"

// 関数
// 2Dスプライトを3D回転
inline static DirectX::XMFLOAT2 Rotate3D(DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 center, DirectX::XMFLOAT3 angle_3d)
{
	DirectX::XMVECTOR v_center = DirectX::XMLoadFloat2(&center);
	DirectX::XMVECTOR v_angle_3d = DirectX::XMLoadFloat3(&angle_3d);
	DirectX::XMVECTOR v_pos_3d = { pos.x,pos.y,0.0f };

	v_pos_3d = DirectX::XMVectorSubtract(v_pos_3d, v_center);

	DirectX::XMMATRIX m_angle_3d = DirectX::XMMatrixRotationRollPitchYawFromVector(v_angle_3d);
	v_pos_3d = DirectX::XMVector3Transform(v_pos_3d, m_angle_3d);

	v_pos_3d = DirectX::XMVectorAdd(v_pos_3d, v_center);

	DirectX::XMStoreFloat2(&pos, v_pos_3d);

	return pos;
};

//-------------------------------------------
// Sprite メンバ関数
//-------------------------------------------

// コンストラクタ
Sprite::Sprite(const wchar_t* w_filename, const char* replaced_ps_filename)
{
	Graphics*		graphics	= Graphics::GetInstance();
	ID3D11Device*	device		= graphics->GetDevice().Get();

	// 頂点情報のセット
	const Sprite::Vertex VERTICES[] = {
		{{-0.5,+0.5,0},{1,1,1,1},{0,0}},
		{{+0.5,+0.5,0},{1,0,0,1},{1,0}},
		{{-0.5,-0.5,0},{0,1,0,1},{0,1}},
		{{+0.5,-0.5,0},{0,0,1,1},{1,1}},
	};

	// 頂点バッファオブジェクトの生成
	D3D11_BUFFER_DESC buffer_desc = {};
	buffer_desc.ByteWidth			= sizeof(VERTICES);
	buffer_desc.Usage				= D3D11_USAGE_DYNAMIC;
	buffer_desc.BindFlags			= D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags			= 0;
	buffer_desc.StructureByteStride = 0;

	// サブリソースデータを作成し、頂点バッファ作成の際に使用
	D3D11_SUBRESOURCE_DATA subresource_data = {};
	subresource_data.pSysMem			= VERTICES;
	subresource_data.SysMemPitch		= 0;
	subresource_data.SysMemSlicePitch	= 0;

	HRESULT hr = S_OK;
	hr = device->CreateBuffer(&buffer_desc, &subresource_data, &vertex_buffer);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// 入力レイアウト要素の追加
	D3D11_INPUT_ELEMENT_DESC input_element_desc[] = {
		{"POSITION",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0},
		{"COLOR",		0,	DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0},
		{"TEXCOORD",	0,	DXGI_FORMAT_R32G32_FLOAT,		0,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0},
	};

	// テクスチャ読み込み
	hr = TextureLoader::LoadTextureFromFile(w_filename, &shader_resource_view, &texture2d_desc);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// シェーダー作成
	hr = Shader::CreateVSFromCso("shader/2d_base_vs.cso", vertex_shader.GetAddressOf(), input_layout.GetAddressOf(), input_element_desc, _countof(input_element_desc));
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	if(replaced_ps_filename)
		hr = Shader::CreatePSFromCso(replaced_ps_filename, pixel_shader.GetAddressOf());
	else
		hr = Shader::CreatePSFromCso("shader/2d_base_ps.cso", pixel_shader.GetAddressOf());

	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

// コンストラクタ(SRVを使用)
Sprite::Sprite(ID3D11ShaderResourceView* shader_resource_view, const char* replaced_ps_filename)
{	
	Graphics* graphics = Graphics::GetInstance();
	ID3D11Device* device = graphics->GetDevice().Get();

	// 頂点情報のセット
	Vertex vertices[] = {
		{{-0.5,+0.5,0},{1,1,1,1},{0,0}},
		{{+0.5,+0.5,0},{1,0,0,1},{1,0}},
		{{-0.5,-0.5,0},{0,1,0,1},{0,1}},
		{{+0.5,-0.5,0},{0,0,1,1},{1,1}},
	};

	// 頂点バッファオブジェクトの生成
	D3D11_BUFFER_DESC buffer_desc = {};
	buffer_desc.ByteWidth			= sizeof(vertices);
	buffer_desc.Usage				= D3D11_USAGE_DYNAMIC;
	buffer_desc.BindFlags			= D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags			= 0;
	buffer_desc.StructureByteStride = 0;

	// サブリソースデータを作成し、頂点バッファ作成の際に使用
	D3D11_SUBRESOURCE_DATA subresource_data = {};
	subresource_data.pSysMem				= vertices;
	subresource_data.SysMemPitch			= 0;
	subresource_data.SysMemSlicePitch	= 0;
	HRESULT hr = S_OK;
	hr = device->CreateBuffer(&buffer_desc, &subresource_data, &vertex_buffer);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// シェーダーリソースを使用している時
	if (shader_resource_view)
	{
		is_load_file = false;
		shader_resource_view->AddRef();
		this->shader_resource_view = shader_resource_view;

		ComPtr<ID3D11Resource> resource;
		this->shader_resource_view->GetResource(resource.GetAddressOf());
		ComPtr<ID3D11Texture2D> texture2d;
		resource.Get()->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
		texture2d->GetDesc(&texture2d_desc);
	}

	// インプットエレメント
	D3D11_INPUT_ELEMENT_DESC input_element_desc[] = {
		{"POSITION",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0},
		{"COLOR",		0,	DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0},
		{"TEXCOORD",	0,	DXGI_FORMAT_R32G32_FLOAT,		0,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0},
	};

	// シェーダー作成
	hr = Shader::CreateVSFromCso("shader/2d_base_vs.cso", vertex_shader.GetAddressOf(), 
		input_layout.GetAddressOf(), input_element_desc, _countof(input_element_desc));
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	if (replaced_ps_filename)
		hr = Shader::CreatePSFromCso(replaced_ps_filename, pixel_shader.GetAddressOf());
	else
		hr = Shader::CreatePSFromCso("shader/2d_base_ps.cso", pixel_shader.GetAddressOf());

	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

// そのまま描画
void Sprite::Render(DirectX::XMFLOAT2 d_pos, DirectX::XMFLOAT2 d_size,
	DirectX::XMFLOAT4 color,
	DirectX::XMFLOAT3 angle_3d,
	DirectX::XMFLOAT2 t_pos, DirectX::XMFLOAT2 t_size)
{
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	// スクリーンのサイズを取得
	D3D11_VIEWPORT	viewport		= {};
	UINT			num_viewports	= 1;

	device_context->RSGetViewports(&num_viewports, &viewport);
	// このメンバ関数の引数から矩形の各頂点の位置を計算
	// left-top
	DirectX::XMFLOAT2 pos0 = d_pos;

	//right-top
	DirectX::XMFLOAT2 pos1 = { d_pos.x + d_size.x,d_pos.y };

	// left-bottom
	DirectX::XMFLOAT2 pos2 = { d_pos.x,d_pos.y + d_size.y };

	//right-bottom
	DirectX::XMFLOAT2 pos3 = { d_pos.x + d_size.x,d_pos.y + d_size.y };

	//回転の中心を矩形の中心点にした場合
	DirectX::XMFLOAT2 center = { d_pos.x + d_size.x * 0.5f , d_pos.y + d_size.y * 0.5f };
	pos0 = Rotate3D(pos0, center, angle_3d);
	pos1 = Rotate3D(pos1, center, angle_3d);
	pos2 = Rotate3D(pos2, center, angle_3d);
	pos3 = Rotate3D(pos3, center, angle_3d);

	// スクリーン座標系からNDCへの座標変換
	pos0 = { 2.0f * pos0.x / viewport.Width - 1.0f ,
		1.0f - 2.0f * pos0.y / viewport.Height };

	pos1 = { 2.0f * pos1.x / viewport.Width - 1.0f ,
		1.0f - 2.0f * pos1.y / viewport.Height };

	pos2 = { 2.0f * pos2.x / viewport.Width - 1.0f ,
		1.0f - 2.0f * pos2.y / viewport.Height };

	pos3 = { 2.0f * pos3.x / viewport.Width - 1.0f ,
		1.0f - 2.0f * pos3.y / viewport.Height };

	// シェーダーのバインド
	device_context->VSSetShader(vertex_shader.Get(), nullptr, 0);
	device_context->PSSetShader(pixel_shader.Get(), nullptr, 0);

	// シェーダーリソースのバインド
	device_context->PSSetShaderResources(0, 1, shader_resource_view.GetAddressOf());

	// 定数バッファのバインド

	// 頂点バッファオブジェクトを更新
	// テクスチャ座標を頂点バッファにセット
	D3D11_MAPPED_SUBRESOURCE mapped_subresource;
	device_context->Map(vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);

	if (t_size.x <= 0.0f && t_size.y <= 0.0f)
	{
		t_size.x = (float)texture2d_desc.Width;
		t_size.y = (float)texture2d_desc.Height;
	}

	float texcoord_x_start	= (t_pos.x / (float)texture2d_desc.Width);
	float texcoord_x_end	= (t_pos.x / (float)texture2d_desc.Width) + (t_size.x / (float)texture2d_desc.Width);
	float texcoord_y_start	= (t_pos.y / (float)texture2d_desc.Height);
	float texcoord_y_end	= (t_pos.y / (float)texture2d_desc.Height) + (t_size.y / (float)texture2d_desc.Height);

	// 頂点設定
	Vertex* vertices = RCast(Vertex*, mapped_subresource.pData);
	if (vertices)
	{
		vertices[0].position = { pos0.x,pos0.y,0 };
		vertices[1].position = { pos1.x,pos1.y,0 };
		vertices[2].position = { pos2.x,pos2.y,0 };
		vertices[3].position = { pos3.x,pos3.y,0 };
		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = color;

		vertices[0].texcoord = { texcoord_x_start,	texcoord_y_start };
		vertices[1].texcoord = { texcoord_x_end,	texcoord_y_start };
		vertices[2].texcoord = { texcoord_x_start,	texcoord_y_end };
		vertices[3].texcoord = { texcoord_x_end,	texcoord_y_end };
	}
	device_context->Unmap(vertex_buffer.Get(), 0);

	// 頂点バッファセット
	UINT stride = sizeof(Vertex);
	UINT offset = 0 ;
	device_context->IASetVertexBuffers(0, 1, vertex_buffer.GetAddressOf(), &stride, &offset);

	// プリミティブタイプおよびデータの順序に関する情報のバインド
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// 入力レイアウトオブジェクトのバインド
	device_context->IASetInputLayout(input_layout.Get());

	// プリミティブの描画
	device_context->Draw(4, 0);
}

// スプライトをテキストとして描画
void Sprite::RenderText(std::string str, DirectX::XMFLOAT2 d_pos, DirectX::XMFLOAT2 d_size, DirectX::XMFLOAT4 color)
{
	size_t str_count = 0;
	while (str[str_count])
	{
		Render({ d_pos.x,d_pos.y }, d_size, color);
		str_count++;
	}
}