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
	virtual GameModel*			GetModel()					{ return nullptr; }

	// public:�Z�b�^�[�֐�
	void SetPosition(const DirectX::XMFLOAT3& f_position)	{ this->translation = f_position; }
	void SetAngle(const DirectX::XMFLOAT3& f_angle)			{ this->rotation = f_angle; }
	void SetScale(const DirectX::XMFLOAT3& f_scale)			{ this->scaling = f_scale; }

protected:

	// protected:�ϐ�
	bool				accumulate_particles	= true;
	bool				integrate_particles		= false;
	float				object_time				= 0.0f;
	DirectX::XMFLOAT3	translation				= { 0,0,0 };
	DirectX::XMFLOAT3	rotation				= { 0,0,0 };
	DirectX::XMFLOAT3	scaling					= { 1,1,1 };

	DirectX::XMFLOAT4X4	transform = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};
};

#endif // __GAME_OBJECT_H__