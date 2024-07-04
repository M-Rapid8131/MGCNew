// <>�C���N���[�h
#include <crtdbg.h>
#include <imgui.h>

// ""�C���N���[�h
// LightBlueEngine
#include "graphics.h"
#include "engine_utility.h"
#include "shader.h"
#include "particle_system.h"

// �Q�[���\�[�X�t�@�C��
#include "misc.h"

//--------------------------------------
//	ParticleSystem �����o�֐�
//-------------------------------------

// �R���X�g���N�^
ParticleSystem::ParticleSystem(CbParticleEmitter emitter_setting, bool use_accumulate_ps,const char* accumulate_ps_csoname)
{
	Graphics*				graphics		= Graphics::GetInstance();
	std::lock_guard<std::mutex> lock(graphics->GetMutex());
	ID3D11Device*			device			= graphics->GetDevice().Get();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	HRESULT	hr = S_OK;

	particle_emitter_constants = emitter_setting;

	D3D11_BUFFER_DESC buffer_desc	= {};

	// �o�b�t�@����
	// �o�b�t�@�쐬�̂��߂̐ݒ�I�v�V����
	buffer_desc.ByteWidth 
		= SCast(UINT, sizeof(SbParticle) * particle_emitter_constants.emit_amounts);
	buffer_desc.Usage				= D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags			= D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	buffer_desc.CPUAccessFlags		= D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags			= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	buffer_desc.StructureByteStride = sizeof(SbParticle);

	hr = device->CreateBuffer(&buffer_desc, nullptr, particle_sbuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// �萔�o�b�t�@�ݒ�
	buffer_desc.ByteWidth			= SCast(UINT, sizeof(CbParticle));
	buffer_desc.Usage				= D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags		= 0;
	buffer_desc.MiscFlags			= 0;
	buffer_desc.StructureByteStride = 0;

	hr = device->CreateBuffer(&buffer_desc, nullptr, particle_cbuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_SUBRESOURCE_DATA subresource_data = {};
	subresource_data.pSysMem = &particle_emitter_constants;
	subresource_data.SysMemPitch = 0;
	subresource_data.SysMemSlicePitch = 0;

	buffer_desc.ByteWidth = SCast(UINT, sizeof(CbParticleEmitter));
	hr = device->CreateBuffer(&buffer_desc, &subresource_data, particle_emitter_cbuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	buffer_desc.ByteWidth = SCast(UINT, sizeof(CbParticleFlock));
	hr = device->CreateBuffer(&buffer_desc, nullptr, particle_flock_cbuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// �V�F�[�_�[���\�[�X�r���[�쐬
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Format					= DXGI_FORMAT_UNKNOWN;
	srv_desc.ViewDimension			= D3D11_SRV_DIMENSION_BUFFER;
	srv_desc.Buffer.ElementOffset	= 0;
	srv_desc.Buffer.NumElements		= SCast(UINT, particle_emitter_constants.emit_amounts);

	hr = device->CreateShaderResourceView(
		particle_sbuffer.Get(),
		&srv_desc,
		shader_resource_view.GetAddressOf()
	);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// UAV����
	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
	uav_desc.Format					= DXGI_FORMAT_UNKNOWN;
	uav_desc.ViewDimension			= D3D11_UAV_DIMENSION_BUFFER;
	uav_desc.Buffer.FirstElement	= 0;
	uav_desc.Buffer.Flags			= 0;
	uav_desc.Buffer.NumElements		
		= SCast(UINT, particle_emitter_constants.emit_amounts);

	hr = device->CreateUnorderedAccessView(
		particle_sbuffer.Get(),
		&uav_desc,
		unordered_access_view.GetAddressOf()
	);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// �V�F�[�_�[
	hr = Shader::CreateVSFromCso("shader/particle_vs.cso", vertex_shader.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = Shader::CreatePSFromCso("shader/particle_ps.cso", pixel_shader.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = Shader::CreateGSFromCso("shader/particle_gs.cso", geometry_shader.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = Shader::CreateCSFromCso("shader/particle_cs.cso", compute_shader.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = Shader::CreateCSFromCso("shader/particle_initializer_cs.cso", initializer_compute_shader.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));


	if(use_accumulate_ps)
	{
		// ���f���ɉ����ăp�[�e�B�N����������ꍇ�͂����ŏ�����
		particle_constants.accel_attenuation	= DEFAULT_ACCEL_ATTENUATION;

		// count_buffer
		buffer_desc.ByteWidth			= SCast(UINT, sizeof(uint32_t));
		buffer_desc.Usage				= D3D11_USAGE_DEFAULT;
		buffer_desc.BindFlags			= D3D11_BIND_UNORDERED_ACCESS;
		buffer_desc.CPUAccessFlags		= D3D11_CPU_ACCESS_READ;
		buffer_desc.MiscFlags			= 0;
		buffer_desc.StructureByteStride = SCast(UINT, sizeof(uint32_t));

		hr = device->CreateBuffer(&buffer_desc, NULL, particle_count_buffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		// append_buffer_uav
		uav_desc.Format					= DXGI_FORMAT_UNKNOWN;
		uav_desc.ViewDimension			= D3D11_UAV_DIMENSION_BUFFER;
		uav_desc.Buffer.FirstElement	= 0;
		uav_desc.Buffer.Flags			= D3D11_BUFFER_UAV_FLAG_APPEND;
		uav_desc.Buffer.NumElements		= particle_emitter_constants.emit_amounts;

		hr = device->CreateUnorderedAccessView(
			particle_sbuffer.Get(),
			&uav_desc,
			append_buffer_uav.GetAddressOf()
		);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		// accumulate_pixel_shader
		std::string cso_name = "shader/";

		cso_name.append(accumulate_ps_csoname);
		hr = Shader::CreatePSFromCso(cso_name.c_str(), accumulate_pixel_shader.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	// �p�[�e�B�N���̏����ݒ��CS���s��

	// �R���X�^���g�o�b�t�@
	device_context->UpdateSubresource(particle_emitter_cbuffer.Get(), 0, nullptr, &emitter_setting, 0, 0);
	device_context->CSSetConstantBuffers(SCast(UINT, EnumBufferGPUParticle::PARTICLE_EMITTER), 1, particle_emitter_cbuffer.GetAddressOf());

	// UAV���Z�b�g
	device_context->CSSetUnorderedAccessViews(0, 1, unordered_access_view.GetAddressOf(), nullptr);

	device_context->CSSetShader(initializer_compute_shader.Get(), nullptr, 0);

	UINT num_threads = Align(particle_emitter_constants.emit_amounts, 512);
	// �v�Z�V�F�[�_�[���s
	device_context->Dispatch(num_threads / 512, 1, 1);

	// ���UAV���Z�b�g
	ID3D11UnorderedAccessView* null_uav = nullptr;
	device_context->CSSetUnorderedAccessViews(0, 1, &null_uav, NULL);
}

// �X�V����
void ParticleSystem::Update(float elapsed_time, ID3D11ComputeShader* replaced_cs)
{
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	if (particle_emitter_constants.emit_amounts == 0)
	{
		return;
	}

	particle_constants.delta_time	=	elapsed_time;
	particle_constants.noise_gap	=	rand() % particle_emitter_constants.emit_amounts;

	// �p�[�e�B�N���ړ������̐��K��
	DirectX::XMVECTOR v_emit_direction
		= DirectX::XMLoadFloat4(&particle_emitter_constants.emit_direction);

	v_emit_direction = DirectX::XMVector3Normalize(v_emit_direction);

	DirectX::XMStoreFloat4(&particle_emitter_constants.emit_direction,
		v_emit_direction);

	// UAV���Z�b�g
	device_context->CSSetUnorderedAccessViews(0, 1, unordered_access_view.GetAddressOf(), nullptr);

	// �R���X�^���g�o�b�t�@
	device_context->UpdateSubresource(particle_cbuffer.Get(), 0, 0, &particle_constants, 0, 0);
	device_context->UpdateSubresource(particle_emitter_cbuffer.Get(), 0, 0, &particle_emitter_constants, 0, 0);
	device_context->UpdateSubresource(particle_flock_cbuffer.Get(), 0, 0, &particle_flock_constants, 0, 0);
	device_context->CSSetConstantBuffers(SCast(UINT, EnumBufferGPUParticle::PARTICLE), 1, particle_cbuffer.GetAddressOf());
	device_context->CSSetConstantBuffers(SCast(UINT, EnumBufferGPUParticle::PARTICLE_EMITTER), 1, particle_emitter_cbuffer.GetAddressOf());
	device_context->CSSetConstantBuffers(SCast(UINT, EnumBufferGPUParticle::PARTICLE_FLOCK), 1, particle_flock_cbuffer.GetAddressOf());

	device_context->CSSetShader(replaced_cs ? replaced_cs : compute_shader.Get(), NULL, 0);

	UINT num_threads = Align(particle_emitter_constants.emit_amounts, 256);

	// �v�Z�V�F�[�_�[���s
	device_context->Dispatch(num_threads / 512, 1, 1);

	// ���UAV���Z�b�g
	ID3D11UnorderedAccessView* null_uav = nullptr;
	device_context->CSSetUnorderedAccessViews(0, 1, &null_uav, NULL);
}

// ImGui�\��
void ParticleSystem::DebugGUI()
{
	if (ImGui::CollapsingHeader("ParticleSystem"))
	{
		ImGui::DragFloat("Accel Attenuation", &particle_constants.accel_attenuation, 0.01f,-2.0f,2.0f);
		ImGui::Separator();
		ImGui::DragFloat3("Emit Position", &particle_emitter_constants.emit_position.x, 0.05f);
		ImGui::DragFloat3("Emit Amplitude", &particle_emitter_constants.emit_amplitude.x, 0.05f);
		ImGui::DragFloat3("Emit Direction", &particle_emitter_constants.emit_direction.x, 0.05f);
		ImGui::ColorEdit3("Emit Color", &particle_emitter_constants.emit_color.x);
		ImGui::DragFloat("Spread Rate", &particle_emitter_constants.spread_rate, 0.05f, 0.0f, 1.0f);
		ImGui::DragFloat("Emit Size", &particle_emitter_constants.emit_size, 0.05f, 0.0f, 1.0f);
		ImGui::DragFloat("Emit Speed", &particle_emitter_constants.emit_speed, 0.05f, 0.0f, 10.0f);
		ImGui::DragFloat("Emit Accel", &particle_emitter_constants.emit_accel, 0.05f, 0.0f, 10.0f);
		ImGui::DragFloat("Life Time", &particle_emitter_constants.life_time, 0.05f, 0.1f, 10.0f);
		ImGui::DragFloat("Start Diff", &particle_emitter_constants.start_diff, 0.05f, 0.05f, 10.0f);
	}
}

// �`�揈��
void ParticleSystem::Render(ID3D11PixelShader* replace_ps)
{
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	// �V�F�[�_�[���Z�b�g
	device_context->VSSetShader(vertex_shader.Get(),							nullptr, 0);
	device_context->PSSetShader(replace_ps ? replace_ps : pixel_shader.Get(),	nullptr, 0);
	device_context->GSSetShader(geometry_shader.Get(),							nullptr, 0);

	device_context->UpdateSubresource(particle_cbuffer.Get(), 0, 0, &particle_constants, 0, 0);
	device_context->VSSetConstantBuffers(SCast(UINT, EnumBufferGPUParticle::PARTICLE), 1, particle_cbuffer.GetAddressOf());
	device_context->GSSetConstantBuffers(SCast(UINT, EnumBufferGPUParticle::PARTICLE), 1, particle_cbuffer.GetAddressOf());
	device_context->PSSetConstantBuffers(SCast(UINT, EnumBufferGPUParticle::PARTICLE), 1, particle_cbuffer.GetAddressOf());
	
	device_context->UpdateSubresource(particle_emitter_cbuffer.Get(), 0, 0, &particle_emitter_constants, 0, 0);
	device_context->VSSetConstantBuffers(SCast(UINT, EnumBufferGPUParticle::PARTICLE_EMITTER), 1, particle_emitter_cbuffer.GetAddressOf());
	device_context->GSSetConstantBuffers(SCast(UINT, EnumBufferGPUParticle::PARTICLE_EMITTER), 1, particle_emitter_cbuffer.GetAddressOf());
	device_context->PSSetConstantBuffers(SCast(UINT, EnumBufferGPUParticle::PARTICLE_EMITTER), 1, particle_emitter_cbuffer.GetAddressOf());
	
	device_context->GSSetShaderResources(1, 1, shader_resource_view.GetAddressOf());

	device_context->IASetInputLayout(NULL);
	device_context->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	device_context->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT,0);
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	device_context->DrawInstanced(INSTANCE_COUNT, particle_emitter_constants.emit_amounts, 0, 0);

	// �V�F�[�_�[��nullptr���Z�b�g
	device_context->VSSetShader(nullptr, nullptr, 0);	
	device_context->PSSetShader(nullptr, nullptr, 0);
	device_context->GSSetShader(nullptr, nullptr, 0);
	device_context->CSSetShader(nullptr, nullptr, 0);
	
	// ��̃V�F�[�_�[���\�[�X�r���[���Z�b�g
	ID3D11ShaderResourceView* null_srv = nullptr;
	device_context->GSSetShaderResources(1, 1, &null_srv);
}

void ParticleSystem::AddParticle(UINT amount)
{
	Graphics*		graphics	= Graphics::GetInstance();
	ID3D11Device*	device		= graphics->GetDevice().Get();

	HRESULT	hr = S_OK;

	particle_emitter_constants.emit_amounts += amount;
	max_groups++;

	D3D11_BUFFER_DESC buffer_desc = {};

	// �o�b�t�@����
	// �o�b�t�@�쐬�̂��߂̐ݒ�I�v�V����
	buffer_desc.ByteWidth 
		= SCast(UINT, sizeof(SbParticle) * particle_emitter_constants.emit_amounts);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	buffer_desc.StructureByteStride = sizeof(SbParticle);

	hr = device->CreateBuffer(&buffer_desc, NULL, particle_sbuffer.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// �V�F�[�_�[���\�[�X�r���[�쐬
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Format = DXGI_FORMAT_UNKNOWN;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srv_desc.Buffer.ElementOffset = 0;
	srv_desc.Buffer.NumElements 
		= SCast(UINT, particle_emitter_constants.emit_amounts);

	hr = device->CreateShaderResourceView(
		particle_sbuffer.Get(),
		&srv_desc,
		shader_resource_view.ReleaseAndGetAddressOf()
	);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// UAV����
	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
	uav_desc.Format = DXGI_FORMAT_UNKNOWN;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uav_desc.Buffer.FirstElement = 0;
	uav_desc.Buffer.Flags = 0;
	uav_desc.Buffer.NumElements = SCast(UINT, particle_emitter_constants.emit_amounts);

	hr = device->CreateUnorderedAccessView(
		particle_sbuffer.Get(),
		&uav_desc,
		unordered_access_view.ReleaseAndGetAddressOf()
	);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

// ���f���ɉ����ăp�[�e�B�N���𐶐�
void ParticleSystem::AccumulateParticles(DrawCallback draw_callback)
{
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	HRESULT hr = S_OK;

	// �ꎞ�ۑ��p�ϐ�
	ComPtr<ID3D11RenderTargetView>		cached_rtv;
	ComPtr<ID3D11DepthStencilView>		cached_dsv;
	ComPtr<ID3D11UnorderedAccessView>	cached_uav;

	// rtv�Adsv�Auav����U�ړ�
	device_context->OMGetRenderTargetsAndUnorderedAccessViews(
		1, cached_rtv.GetAddressOf(), cached_dsv.GetAddressOf(),
		1, 1, cached_uav.GetAddressOf()
	);

	// uav�ȊO����ɂ���Auav�͕ʂ̕ϐ��ɍ����ւ���
	ID3D11RenderTargetView* null_rtv = {};
	UINT initial_count = 0;
	device_context->OMSetRenderTargetsAndUnorderedAccessViews(
		1, &null_rtv, NULL,
		1, 1, append_buffer_uav.GetAddressOf(), &initial_count
	);

	draw_callback(accumulate_pixel_shader.Get());

	// rtv�Adsv�Auav�����ɖ߂�
	device_context->OMSetRenderTargetsAndUnorderedAccessViews(
		1, cached_rtv.GetAddressOf(), cached_dsv.Get(),
		1, 1, cached_uav.GetAddressOf(),NULL
	);

	// �o�b�t�@����f�[�^���R�s�[
	device_context->CopyStructureCount(particle_count_buffer.Get(), 0, append_buffer_uav.Get());

	// mapped_subresource�ɓW�J
	D3D11_MAPPED_SUBRESOURCE mapped_subresource = {};
	hr = device_context->Map(particle_count_buffer.Get(), 0, D3D11_MAP_READ, 0, &mapped_subresource);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// �f�[�^�̃|�C���^����p�[�e�B�N�������擾
	UINT count = *RCast(UINT*, mapped_subresource.pData);

	// �W�J�����f�[�^�����ɖ߂�
	device_context->Unmap(particle_count_buffer.Get(), 0);

	// �p�[�e�B�N������萔�o�b�t�@�p�̃f�[�^�Ɋi�[
	particle_emitter_constants.emit_amounts = count;
}

// �I������
void ParticleSystem::Uninitialize()
{
	vertex_shader.Reset();
	geometry_shader.Reset();
	pixel_shader.Reset();
	compute_shader.Reset();

	accumulate_pixel_shader.Reset();
}
