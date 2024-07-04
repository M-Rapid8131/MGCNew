#ifndef __RANDOM_NOISE_H__
#define __RANDOM_NOISE_H__

// <>�C���N���[�h
#include <wrl.h>
#include <memory>
#include <array>
#include <vector>
#include <d3d11.h>

// ""�C���N���[�h
// LightBlueEngine
#include "singleton.h"

// using
using Microsoft::WRL::ComPtr;

// class >> [RandomNoise]
class RandomNoise
{
private:
	// private:�萔
	static const UINT	DEFAULT_SQUARE = 128;

public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	RandomNoise(UINT width = DEFAULT_SQUARE, UINT height = DEFAULT_SQUARE);
	~RandomNoise() {};

	// public:�ʏ�֐�
	void MakeNoise();
	void DebugGUI();

	// public:�Q�b�^�[�֐�
	const UINT& GetNoiseWidth()		{ return noise_width; }
	const UINT& GetNoiseHeight()	{ return noise_height; }

	const std::vector<int>& GetNoiseData(size_t index) { return noise_data[index]; }

private:
	// private:�ϐ�
	UINT								noise_width		= 0;
	UINT								noise_height	= 0;
	UINT								noise_size		= 0;
	UINT								noise_seed		= 0;
	ComPtr<ID3D11Texture2D>				noise_texture;
	ComPtr<ID3D11ShaderResourceView>	noise_srv;

	std::unique_ptr<DWORD[]>			noise_color;
	std::array<std::vector<int>,3>		noise_data;
};

#endif // __RANDOM_NOISE_H__