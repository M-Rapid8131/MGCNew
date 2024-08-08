#ifndef __COMPONENT_BASE_H__
#define __COMPONENT_BASE_H__

// <>�C���N���[�h
#include <vector>
#include <any>
#include <DirectXMath.h>

// class >> [Component]
// �R���|�[�l���g�̌��ƂȂ�N���X�B
template<class Type>
class Component
{
public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	template<class Type> Component(Type init_data) : data(init_data) = 0;
	virtual ~Component() {};

	// public:�Q�b�^�[�֐�
	template<class Type> virtual const Type& Get() = 0;
	template<class Type> virtual Type& GetRef() = 0;

	// public:�Z�b�^�[�֐�
	template<class Type> virtual void Set(const Type& new_data) { data = new_data; };

protected:
	std::any data;
};

#endif // __COMPONENT_BASE_H__

