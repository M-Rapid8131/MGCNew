#ifndef __NUMBER_RENDERER_H__
#define __NUMBER_RENDERER_H__

// <>ƒCƒ“ƒNƒ‹[ƒh
#include <memory>

// ""ƒCƒ“ƒNƒ‹[ƒh
// LightBlueEngine
#include "2D/sprite.h"
#include "singleton.h"

// enum
enum class EnumNumberAlignment
{
	LEFT_ALIGNMENT,		// ¶‘µ‚¦
	CENTER_ALIGNMENT,	// ’†‰›‘µ‚¦
	RIGHT_ALIGNMENT,	// ‰E
};

// int—p—İæŠÖ”
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
// ”š‚ğ•`‰æ‚·‚éƒNƒ‰ƒXBSpriteƒNƒ‰ƒX‚ğg—pB
class NumberRenderer
{
private:
	// private:’è”
	const int	MINUS_POS		= 10;
	const int	PERIOD_POS		= 11;
	const float TEXT_SPR_WIDTH	= 120.0f;
	const float TEXT_SPR_HEIGHT	= 150.0f;

public:
	// public:ƒRƒ“ƒXƒgƒ‰ƒNƒ^EƒfƒXƒgƒ‰ƒNƒ^
	NumberRenderer();
	~NumberRenderer();

	// public:’ÊíŠÖ”
	void RenderInt(int number, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 scale,
		EnumNumberAlignment alignment = EnumNumberAlignment::LEFT_ALIGNMENT);
	//	RenderInt‚É‚Â‚¢‚Ä
	//		‚T‚S‚R‚Q‚P(Œ…–Ú) ->digits	
	//		«««««					
	//		‚P‚Q‚R‚S‚T ->number			

	void RenderFloat(float number, int decimal_points, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 scale, 
		EnumNumberAlignment alignment = EnumNumberAlignment::LEFT_ALIGNMENT);
	//	RenderFloat‚É‚Â‚¢‚Ä
	//		digits->‚W‚V‚U‚T‚S@‚R‚Q‚P(Œ…–Ú)
	//				«««««	‚P‚Q‚R(¬”“_‘æNˆÊ)	<-decimal_points
	//				«««««	«««
	// 				‚P‚Q‚R‚S‚TD‚U‚V‚W ->number			

private:
	// private:•Ï”
	std::unique_ptr<Sprite>		number_sprite;
};

#endif // __NUMBER_RENDERER_H__