#ifndef __COMPONENT_PRESETS_H__
#define __COMPONENT_PRESETS_H__

// ""�C���N���[�h
#include "component_base.h"

// class >> [Translate] �p���FComponent<DirectX::XMFLOAT3>
// �I�u�W�F�N�g�̈ړ����Ǘ�����R���|�[�l���g�B
class Translate : public Component<DirectX::XMFLOAT3>
{
public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	Translate(DirectX::XMFLOAT3 init_data) : Component<DirectX::XMFLOAT3>(init_data) {};
	~Translate() {};

	// public:�ʏ�֐�
	void AddImpulse(DirectX::XMFLOAT3 impulse) 
	{
		DirectX::XMFLOAT3& f3_data = GetRef();
		f3_data.x += impulse.x;
		f3_data.y += impulse.y;
		f3_data.z += impulse.z;
	}

	// public:�Q�b�^�[�֐�
	const DirectX::XMFLOAT3&	Get()		{ return std::any_cast<const DirectX::XMFLOAT3&>(data); }
	DirectX::XMFLOAT3&			GetRef()	{ return std::any_cast<DirectX::XMFLOAT3&>(data); }

	// public:�Z�b�^�[�֐�
	void Set(const DirectX::XMFLOAT3& new_data) { Component::Set<DirectX::XMFLOAT3>(new_data); }
};

// class >> [Scale] �p���FComponent<DirectX::XMFLOAT3>
// �I�u�W�F�N�g�̃T�C�Y���Ǘ�����R���|�[�l���g�B
class Scale : public Component<DirectX::XMFLOAT3>
{
public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	Scale(DirectX::XMFLOAT3 init_data) : Component<DirectX::XMFLOAT3>(init_data) {};
	~Scale() {};

	// public:�Q�b�^�[�֐�
	const DirectX::XMFLOAT3&	Get()		{ return std::any_cast<const DirectX::XMFLOAT3&>(data); }
	DirectX::XMFLOAT3&			GetRef()	{ return std::any_cast<DirectX::XMFLOAT3&>(data); }

	// public:�Z�b�^�[�֐�
	void Set(const DirectX::XMFLOAT3& new_data) { Component::Set<DirectX::XMFLOAT3>(new_data); }
};

#endif // __COMPONENT_PRESETS_H__

