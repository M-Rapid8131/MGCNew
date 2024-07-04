#ifndef __ENGINE_UTILITY_H__
#define __ENGINE_UTILITY_H__

// <>�C���N���[�h
#include <string>
#include <DirectXMath.h>
#include <magic_enum.hpp>
#include <any>

// �}�N��
// static_cast�Z�k
#define SCast(type,num)		static_cast<type>(num)

// const_cast�Z�k
#define CCast(type,num)		const_cast<type>(num)

// reinterpret_cast�Z�k
#define RCast(type,num)		reinterpret_cast<type>(num)

// any_cast�Z�k
#define ACast(type,num)		std::any_cast<type>(num)

// �v�Z�V�F�[�_�[�p
#define Align(num, alignment) (num + (alignment - 1) & ~(alignment - 1))

// 0~255��0.0~1.0�ɕϊ�
#define NormC(value_255)	value_255 / 255.0f

// �\����
// struct >> [FlagSystem]
template<typename Enum>
struct FlagSystem
{
	// �֐�
	bool GetFlag(Enum item)				// �t���O�������Ă��邩���m�F
	{
		int item_num = magic_enum::enum_integer<Enum>(item);
		return flags & (1 << item_num);
	}

	void	 SetFlag(Enum item, bool flg)	// �t���O��ݒ肷��
	{
		int item_num = magic_enum::enum_integer<Enum>(item);
		flg ? flags |= (1 << item_num) : flags &= ~(1 << item_num);
	}

	void	 SetAllFlag(unsigned int flag)
	{
		flags = flag;
	}

	// �ϐ�
	unsigned int		flags = 0;
};

#endif // __ENGINE_UTILITY_H__