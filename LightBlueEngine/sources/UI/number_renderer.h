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
private:
	// private:�萔
	const int	MINUS_POS		= 10;
	const int	PERIOD_POS		= 11;
	const float TEXT_SPR_WIDTH	= 120.0f;
	const float TEXT_SPR_HEIGHT	= 150.0f;

public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	NumberRenderer();
	~NumberRenderer();

	// public:�ʏ�֐�
	void RenderInt(int number, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 scale,
		EnumNumberAlignment alignment = EnumNumberAlignment::LEFT_ALIGNMENT);
	//	RenderInt�ɂ���
	//		�T�S�R�Q�P(����) ->digits	
	//		����������					
	//		�P�Q�R�S�T ->number			

	void RenderFloat(float number, int decimal_points, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 scale, 
		EnumNumberAlignment alignment = EnumNumberAlignment::LEFT_ALIGNMENT);
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