#ifndef __GPU_PARTICLE_H__
#define __GPU_PARTICLE_H__

// <>インクルード
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <vector>
#include <functional>

// 前方宣言
class Graphics;

// using
using Microsoft::WRL::ComPtr;
using DrawCallback = std::function<void(ID3D11PixelShader*)>;

// enum class
enum class EnumBufferGPUParticle
{
	PARTICLE = 4,
	PARTICLE_EMITTER,
	PARTICLE_FLOCK,
};

// class >> [ParticleSystem]
// パーティクルを生成するクラス。
class ParticleSystem
{
private:
	// private:定数
	static const int INSTANCE_COUNT = 1;

	static constexpr float DEFAULT_POSITION_WEIGHT		= 0.04f;
	static constexpr float DEFAULT_VELOCITY_WEIGHT		= 0.02f;
	static constexpr float DEFAULT_FLEE_WEIGHT			= -0.12f;
	static constexpr float DEFAULT_SIZE					= 0.18f;
	static constexpr float DEFAULT_ACCEL				= 1.0f;
	static constexpr float DEFAULT_ACCEL_ATTENUATION	= 1.0f;
	static constexpr float DEFAULT_LIFE					= 1.0f;

	// private:構造体
	
	// struct >> ParticleSystem >> [SbParticle] register : t1
	struct SbParticle
	{
		UINT				particle_index;
		DirectX::XMFLOAT3	position;
		float				size;
		DirectX::XMFLOAT4	color;
		DirectX::XMFLOAT3	velocity;
		DirectX::XMFLOAT3	acceleration;
		DirectX::XMFLOAT3	normal;
		float				life;
		float				start_time;
	};

	// private:定数バッファ構造体
	
	// struct >> ParticleSystem >> [CbParticle] register : b3
	struct CbParticle
	{
		int					group_count;
		int					noise_gap;

		int					cbparticle_ipad[2];
		
		float				delta_time;
		float				accel_attenuation;

		float				cbparticle_fpad[2];
	};

	// struct >> ParticleSystem >> [CbParticleInitialize] register : b5
	struct CbParticleFlock
	{
		float position_weight	= DEFAULT_POSITION_WEIGHT;
		float velocity_weight	= DEFAULT_VELOCITY_WEIGHT;
		float flee_weight		= DEFAULT_FLEE_WEIGHT;

		float cbparticle_flock_fpad;
	};

public:
	// public:定数バッファ構造体
	
	// struct >> ParticleSystem >> [CbParticleEmitter] register : b4
	struct CbParticleEmitter
	{
		int				emit_amounts = 10000;

		int				cbparticle_emitter_ipad[3];

		DirectX::XMFLOAT4	emit_position	= { 0.0f, 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT4	emit_amplitude	= { 1.0f, 1.0f, 1.0f, 0.0f };
		DirectX::XMFLOAT4	emit_direction	= { 0.0f, 1.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT4	emit_color		= { 1.0f,1.0f,1.0f, 1.0f };
		float				spread_rate		= 1.0f;
		float				emit_size		= DEFAULT_SIZE;
		float				emit_speed		= 1.0f;
		float				emit_accel		= DEFAULT_ACCEL;
		float				life_time		= DEFAULT_LIFE;
		float				start_diff		= 0.5f;

		float				cbparticle_emitter_fpad[2];
	};

	// public:コンストラクタ・デストラクタ
	ParticleSystem(CbParticleEmitter = {}, bool = false, const char* = nullptr);
	~ParticleSystem() {};

	// public:通常関数
	void Update(float, ID3D11ComputeShader* = nullptr);
	void DebugGUI();
	void Render(ID3D11PixelShader* = nullptr);
	void AddParticle(UINT);
	void AccumulateParticles(DrawCallback);
	void Uninitialize();

	// public:ゲッター関数
	CbParticleEmitter& GetCbParticleEmitter() { return particle_emitter_constants; }

private:
	// private:変数
	// シェーダー系
	ComPtr<ID3D11VertexShader>			vertex_shader;
	ComPtr<ID3D11GeometryShader>		geometry_shader;
	ComPtr<ID3D11PixelShader>			pixel_shader;
	ComPtr<ID3D11ComputeShader>			compute_shader;

	// accumulate_psを使用する場合のみ生成
	ComPtr<ID3D11PixelShader>			accumulate_pixel_shader;
	ComPtr<ID3D11ComputeShader>			initializer_compute_shader;

	// シェーダー系以外
	UINT								max_groups;
	ComPtr<ID3D11Buffer>				particle_cbuffer;
	ComPtr<ID3D11Buffer>				particle_emitter_cbuffer;
	ComPtr<ID3D11Buffer>				particle_flock_cbuffer;
	ComPtr<ID3D11Buffer>				particle_sbuffer;
	ComPtr<ID3D11InputLayout>			input_layout;
	ComPtr<ID3D11ShaderResourceView>	shader_resource_view;
	ComPtr<ID3D11UnorderedAccessView>	unordered_access_view;
	CbParticle							particle_constants				= {};
	CbParticleEmitter					particle_emitter_constants		= {};
	CbParticleFlock						particle_flock_constants		= {};

	std::unique_ptr<SbParticle[]>		points;

	// accumulate_psを使用する場合のみ生成
	ComPtr<ID3D11Buffer>				particle_count_buffer;
	ComPtr<ID3D11UnorderedAccessView>	append_buffer_uav;

	// テスト変数
};

#endif // __GPU_PARTICLE_H__