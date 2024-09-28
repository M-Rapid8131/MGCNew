#ifndef __GAME_OBJECT_H__
#define __GAME_OBJECT_H__

// <>�C���N���[�h
#include <d3d11.h>
#include <memory>
#include <vector>
#include <magic_enum.hpp>

// ""�C���N���[�h
// LightBlueEngine
#include "3D/game_model.h"

// class >> [GameObject]
// 3D���f���̃N���X�������Q�[���Ɏg�p���邽�߂̃N���X�B
class GameObject
{
public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	GameObject() {}
	virtual ~GameObject() {}

	// public:�ʏ�֐�
	virtual void	 UpdateTransform();

	// public:�Q�b�^�[�֐�
	const DirectX::XMFLOAT3&	GetTranslation() const	{ return translation; }
	const DirectX::XMFLOAT3&	GetAngle() const		{ return rotation; }
	const DirectX::XMFLOAT3&	GetScale() const		{ return scaling; }
	const DirectX::XMFLOAT4X4&	GetTransform() const		{ return transform; }
	virtual GameModel*			GetModel()					{ return model ? model.get() : nullptr; }

	// public:�Z�b�^�[�֐�
	void SetPosition(const DirectX::XMFLOAT3& new_position)	{ translation = new_position; }
	void SetAngle(const DirectX::XMFLOAT3& new_angle)		{ rotation = new_angle; }
	void SetScale(const DirectX::XMFLOAT3& new_scale)		{ scaling = new_scale; }

protected:

	// protected:�ϐ�
	bool						accumulate_particles	= true;
	bool						integrate_particles		= false;
	float						object_time				= 0.0f;
	DirectX::XMFLOAT3			translation				= { 0,0,0 };
	DirectX::XMFLOAT3			rotation				= { 0,0,0 };
	DirectX::XMFLOAT3			scaling					= { 1,1,1 };
	std::unique_ptr<GameModel>	model;

	DirectX::XMFLOAT4X4			transform = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};
};

#endif // __GAME_OBJECT_H__