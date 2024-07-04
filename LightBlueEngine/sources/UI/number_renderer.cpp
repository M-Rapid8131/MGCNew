// ""インクルード
// LightBlueEngine
#include "engine_utility.h"
#include "number_renderer.h"

// 関数
// 桁数を取得(floatなどの整数部分に使うときはキャストすること)
static int GetDigits(const int value)
{
	std::string str = std::to_string(value);
	return SCast(int, str.size());
}

//-------------------------------------------
// NumberRenderer メンバ関数
//-------------------------------------------

// コンストラクタ
NumberRenderer::NumberRenderer()
{
	number_sprite = std::make_unique<Sprite>(L"resources/sprite/number_spr.png", "shader/2d_base_ps.cso");
}

// デストラクタ
NumberRenderer::~NumberRenderer()
{
	auto ptr = number_sprite.release();
	delete ptr;
}

// 整数描画
void NumberRenderer::RenderInt(int number, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 scale, EnumNumberAlignment alignment)
{
	int count	= 0;	// 描画した数字の個数
	int num		= 0;	// 描画する数字
	int digits  = GetDigits(number);

	DirectX::XMFLOAT2 modified_pos = {0.0f, 0.0f};		// アライメント調整後の位置
	
	switch (alignment)
	{
	case EnumNumberAlignment::LEFT_ALIGNMENT:
		modified_pos = pos;
		break;
	case EnumNumberAlignment::CENTER_ALIGNMENT:
		modified_pos.y = pos.y;
		modified_pos.x = pos.x - (TEXT_SPR_WIDTH * scale.x) * (SCast(float, digits) * 0.5f);
		break;
	case EnumNumberAlignment::RIGHT_ALIGNMENT:
		modified_pos.y = pos.y;
		modified_pos.x = pos.x - (TEXT_SPR_WIDTH * scale.x) * SCast(float, digits);
		break;
	default:
		break;
	}

	// 文字描画
	while (digits > 0)
	{
		num = number / PowInt(10, digits - 1);
		number_sprite->Render(
			{ modified_pos.x + (TEXT_SPR_WIDTH * scale.x) * SCast(float,count),modified_pos.y },
			{ SCast(float, TEXT_SPR_WIDTH) * scale.x,SCast(float, TEXT_SPR_HEIGHT) * scale.y },
			{ 1.0f,1.0f,1.0f,1.0f },
			{ 0.0f,0.0f,0.0f },
			{ TEXT_SPR_WIDTH * SCast(float, num),0.0f },
			{ TEXT_SPR_WIDTH,TEXT_SPR_HEIGHT }
		);
		number = number % PowInt(10, digits - 1);
		digits--;
		count++;
	}
}

// 小数含む数の描画
void NumberRenderer::RenderFloat(float number, int decimal_points, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 scale, EnumNumberAlignment alignment)
{
	int count	= 0;	// 描画した数字の個数
	int adj_num = SCast(int, number) * PowInt(10, decimal_points);		// 小数点を外す
	int num		= 0;	// 描画する数字
	int digits  = GetDigits(adj_num);

	DirectX::XMFLOAT2 modified_pos = {0.0f,0.0f}; // アライメント調整後の位置

	switch (alignment)
	{
	case EnumNumberAlignment::LEFT_ALIGNMENT:
		modified_pos = pos;
		break;
	case EnumNumberAlignment::CENTER_ALIGNMENT:
		modified_pos.y = pos.y;
		modified_pos.x = pos.x - TEXT_SPR_WIDTH * (SCast(float, digits) * 0.5f);
		break;
	case EnumNumberAlignment::RIGHT_ALIGNMENT:
		modified_pos.y = pos.y;
		modified_pos.x = pos.x - TEXT_SPR_WIDTH * SCast(float, digits);
		break;
	default:
		break;
	}

	// 文字描画
	while (digits > 0)
	{
		bool period = (decimal_points == digits) && (num != PERIOD_POS);

		// 小数点を描画するか否か
		if (period)	num = PERIOD_POS;
		else		num = adj_num / PowInt(10, digits - 1);

		number_sprite->Render(
			{ modified_pos.x + (TEXT_SPR_WIDTH * scale.x) * SCast(float,count),modified_pos.y },
			{ SCast(float, TEXT_SPR_WIDTH) * scale.x, SCast(float, TEXT_SPR_HEIGHT) * scale.y },
			{ 1.0f,1.0f,1.0f,1.0f },
			{ 0.0f,0.0f,0.0f },
			{ TEXT_SPR_WIDTH * SCast(float, num),0.0f },
			{ TEXT_SPR_WIDTH,TEXT_SPR_HEIGHT }
		);

		if (!period)
		{
			adj_num = adj_num % PowInt(10, digits - 1);
			digits--;
		}

		count++;
	}
}