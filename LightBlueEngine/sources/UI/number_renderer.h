#ifndef __NUMBER_RENDERER_H__
#define __NUMBER_RENDERER_H__

// <>�C���N���[�h
#include <memory>

// ""�C���N���[�h
// LightBlueEngine
#include "2D/sprite.h"
#include "singleton.h"

// enum
enum class EnumNumberAlignment
{
	LEFT_ALIGNMENT,		// ������
	CENTER_ALIGNMENT,	// ��������
	RIGHT_ALIGNMENT,	// �E
};

// int�p�ݏ�֐�
inline int PowInt(int num, int pow_factor)
{
	int ret = 1;
	for (int i = 0; i < pow_factor; i++)
	{
		ret *= num;
	}
	return ret;
}

// class >> [NumberRenderer]
// ������`�悷��N���X�BSprite�N���X���g�p�B
class NumberRenderer
{
public:
	// public:�萔
	static const int	MINUS_POS		= 10;
	static const int	PERIOD_POS		= 11;
	
	static constexpr float	TEXT_SPR_WIDTH	= 120.0f;
	static constexpr float	TEXT_SPR_HEIGHT	= 200.0f;

	// public:�R���X�g���N�^�E�f�X�g���N�^
	NumberRenderer();
	~NumberRenderer();

	// public:�ʏ�֐�
	void RenderInt(int, DirectX::XMFLOAT2, DirectX::XMFLOAT2, 
		EnumNumberAlignment = EnumNumberAlignment::LEFT_ALIGNMENT, DirectX::XMFLOAT4 = { 1.0f, 1.0f, 1.0f, 1.0f });
	//	RenderInt�ɂ���
	//		�T�S�R�Q�P(����) ->digits	
	//		����������					
	//		�P�Q�R�S�T ->number			

	void RenderFloat(float, int, DirectX::XMFLOAT2, DirectX::XMFLOAT2, 
		EnumNumberAlignment = EnumNumberAlignment::LEFT_ALIGNMENT, DirectX::XMFLOAT4 = { 1.0f, 1.0f, 1.0f, 1.0f });
	//	RenderFloat�ɂ���
	//		digits->�W�V�U�T�S�@�R�Q�P(����)
	//				����������	�P�Q�R(�����_��N��)	<-decimal_points
	//				����������	������
	// 				�P�Q�R�S�T�D�U�V�W ->number			

private:
	// private:�ϐ�
	std::unique_ptr<Sprite>		number_sprite;
};

#endif // __NUMBER_RENDERER_H__