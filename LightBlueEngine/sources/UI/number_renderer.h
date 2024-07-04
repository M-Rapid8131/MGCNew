#ifndef __NUMBER_RENDERER_H__
#define __NUMBER_RENDERER_H__

// <>インクルード
#include <memory>

// ""インクルード
// LightBlueEngine
#include "2D/sprite.h"
#include "singleton.h"

// enum
enum class EnumNumberAlignment
{
	LEFT_ALIGNMENT,		// 左揃え
	CENTER_ALIGNMENT,	// 中央揃え
	RIGHT_ALIGNMENT,	// 右
};

// int用累乗関数
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
// 数字を描画するクラス。Spriteクラスを使用。
class NumberRenderer
{
private:
	// private:定数
	const int	MINUS_POS		= 10;
	const int	PERIOD_POS		= 11;
	const float TEXT_SPR_WIDTH	= 120.0f;
	const float TEXT_SPR_HEIGHT	= 150.0f;

public:
	// public:コンストラクタ・デストラクタ
	NumberRenderer();
	~NumberRenderer();

	// public:通常関数
	void RenderInt(int number, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 scale,
		EnumNumberAlignment alignment = EnumNumberAlignment::LEFT_ALIGNMENT);
	//	RenderIntについて
	//		５４３２１(桁目) ->digits	
	//		↓↓↓↓↓					
	//		１２３４５ ->number			

	void RenderFloat(float number, int decimal_points, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 scale, 
		EnumNumberAlignment alignment = EnumNumberAlignment::LEFT_ALIGNMENT);
	//	RenderFloatについて
	//		digits->８７６５４　３２１(桁目)
	//				↓↓↓↓↓	１２３(小数点第N位)	<-decimal_points
	//				↓↓↓↓↓	↓↓↓
	// 				１２３４５．６７８ ->number			

private:
	// private:変数
	std::unique_ptr<Sprite>		number_sprite;
};

#endif // __NUMBER_RENDERER_H__