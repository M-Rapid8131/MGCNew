#ifndef __FRAMEWORK_H__
#define __FRAMEWORK_H__

// <>�C���N���[�h
#include <vector>
#include <wrl.h>
#include <memory>

// ""�C���N���[�h
// LightBlueEngine
#include "input/game_pad.h"

// using
using namespace std;
using namespace Microsoft::WRL;

// class >> [Framework]
class Framework
{
	// private:�萔
	CONST HWND hwnd;

	// private:�\����

public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	Framework(HWND hwnd) : hwnd(hwnd) {};
	~Framework();

	// public:�R�s�[�R���X�g���N�^�E���[�u�R���X�g���N�^
	Framework(const Framework&)					= delete;
	Framework& operator=(const Framework&)		= delete;
	Framework(Framework&&) noexcept				= delete;
	Framework& operator=(Framework&&) noexcept	= delete;

	// public:�ʏ�֐�
	int					Run();
	LRESULT CALLBACK	HandleMessage(HWND, UINT, WPARAM, LPARAM);
	void				CalculateFrameStats();
	bool				Initialize();
	void				Update(float);
	void				Render();
	bool				Uninitialize();

private:
	// private:�ϐ�
	float				operationg_time	= 0.0f;
	uint32_t			frames			= 0;
};

#endif // __FRAMEWORK_H__