#ifndef __RANDOM_NOISE_H__
#define __RANDOM_NOISE_H__

// <>インクルード
#include <wrl.h>
#include <memory>
#include <array>
#include <vector>
#include <d3d11.h>

// ""インクルード
// LightBlueEngine
#include "singleton.h"

// using
using Microsoft::WRL::ComPtr;

// class >> [RandomNoise]
class RandomNoise
{
private:
	// private:定数
	static const UINT	DEFAULT_SQUARE = 128;

public:
	// public:コンストラクタ・デストラクタ
	RandomNoise(UINT width = DEFAULT_SQUARE, UINT height = DEFAULT_SQUARE);
	~RandomNoise() {};

	// public:通常関数
	void MakeNoise();
	void DebugGUI();

	// public:ゲッター関数
	const UINT& GetNoiseWidth()		{ return noise_width; }
	const UINT& GetNoiseHeight()	{ return noise_height; }

	const std::vector<int>& GetNoiseData(size_t index) { return noise_data[index]; }

private:
	// private:変数
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