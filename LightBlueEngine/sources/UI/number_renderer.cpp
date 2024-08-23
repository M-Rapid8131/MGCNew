// ""�C���N���[�h
// LightBlueEngine
#include "engine_utility.h"
#include "number_renderer.h"

// �֐�
// �������擾(float�Ȃǂ̐��������Ɏg���Ƃ��̓L���X�g���邱��)
static int GetDigits(const int value)
{
	std::string str = std::to_string(value);
	return SCast(int, str.size());
}

//-------------------------------------------
// NumberRenderer �����o�֐�
//-------------------------------------------

// �R���X�g���N�^
NumberRenderer::NumberRenderer()
{
	number_sprite = std::make_unique<Sprite>(L"resources/sprite/number_spr.png", "shader/2d_base_ps.cso");
}

// �f�X�g���N�^
NumberRenderer::~NumberRenderer()
{
	auto ptr = number_sprite.release();
	delete ptr;
}

// �����`��
void NumberRenderer::RenderInt(int number, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 scale, EnumNumberAlignment alignment, DirectX::XMFLOAT4 color, std::string command)
{
	int count	= 0;	// �`�悵�������̌�
	int num		= 0;	// �`�悷�鐔��
	int digits  = GetDigits(number);
	int render_count = 0;
	if (!command.empty())
	{
		digits = 0;
		for (int i = 0; i < command.size(); i++)
		{
			const char cmd = command.at(i);
			if (cmd == 'N')
			{
				digits++;
			}

			render_count++;
		}
		render_count--;
	}

	DirectX::XMFLOAT2 modified_pos = {0.0f, 0.0f};		// �A���C�����g������̈ʒu
	
	switch (alignment)
	{
	case EnumNumberAlignment::LEFT_ALIGNMENT:
		modified_pos = pos;
		break;
	case EnumNumberAlignment::CENTER_ALIGNMENT:
		modified_pos.y = pos.y;
		if(command.empty())
			modified_pos.x = pos.x - (TEXT_SPR_WIDTH * scale.x) * (SCast(float, digits) * 0.4f);
		else
			modified_pos.x = pos.x - (TEXT_SPR_WIDTH * scale.x) * (SCast(float, render_count) * 0.4f);
		break;
	case EnumNumberAlignment::RIGHT_ALIGNMENT:
		modified_pos.y = pos.y;
		if (command.empty())
			modified_pos.x = pos.x - (TEXT_SPR_WIDTH * scale.x) * SCast(float, digits) * 0.8f;
		else
			modified_pos.x = pos.x - (TEXT_SPR_WIDTH * scale.x) * SCast(float, render_count) * 0.8f;
		break;
	default:
		break;
	}

	// �����`��
	if (command.empty())
	{
		while (digits > 0)
		{
			num = number / PowInt(10, digits - 1);
			number_sprite->Render(
				{ modified_pos.x + (TEXT_SPR_WIDTH * scale.x) * (SCast(float,count) * 0.8f), modified_pos.y },
				{ SCast(float, TEXT_SPR_WIDTH) * scale.x, SCast(float, TEXT_SPR_HEIGHT) * scale.y },
				color,
				{ 0.0f,0.0f,0.0f },
				{ TEXT_SPR_WIDTH * SCast(float, num),0.0f },
				{ TEXT_SPR_WIDTH,TEXT_SPR_HEIGHT }
			);
			number = number % PowInt(10, digits - 1);
			digits--;
			count++;
		}
	}
	else
	{
		for (int i = 0; i < command.size(); i++)
		{
			const char cmd = command.at(i);
			switch (cmd)
			{
			case 'N':
				num = number / PowInt(10, digits - 1);
				number_sprite->Render(
					{ modified_pos.x + (TEXT_SPR_WIDTH * scale.x) * SCast(float,count) * 0.8f, modified_pos.y },
					{ SCast(float, TEXT_SPR_WIDTH) * scale.x, SCast(float, TEXT_SPR_HEIGHT) * scale.y },
					color,
					{ 0.0f,0.0f,0.0f },
					{ TEXT_SPR_WIDTH * SCast(float, num),0.0f },
					{ TEXT_SPR_WIDTH,TEXT_SPR_HEIGHT }
				);
				number = number % PowInt(10, digits - 1);
				digits--;
				count++;
				break;
			case 'M':
				number_sprite->Render(
					{ modified_pos.x + (TEXT_SPR_WIDTH * scale.x) * SCast(float,count) * 0.8f, modified_pos.y },
					{ SCast(float, TEXT_SPR_WIDTH) * scale.x, SCast(float, TEXT_SPR_HEIGHT) * scale.y },
					color,
					{ 0.0f,0.0f,0.0f },
					{ TEXT_SPR_WIDTH * SCast(float, MINUS_POS),0.0f },
					{ TEXT_SPR_WIDTH,TEXT_SPR_HEIGHT }
				);
				count++;
				break;
			case 'C':
				number_sprite->Render(
					{ modified_pos.x + (TEXT_SPR_WIDTH * scale.x) * SCast(float,count) * 0.8f, modified_pos.y },
					{ SCast(float, TEXT_SPR_WIDTH) * scale.x, SCast(float, TEXT_SPR_HEIGHT) * scale.y },
					color,
					{ 0.0f,0.0f,0.0f },
					{ TEXT_SPR_WIDTH * SCast(float, COLON_POS),0.0f },
					{ TEXT_SPR_WIDTH,TEXT_SPR_HEIGHT }
				);
				count++;
				break;
			case 'S':
				number_sprite->Render(
					{ modified_pos.x + (TEXT_SPR_WIDTH * scale.x) * SCast(float,count) * 0.8f, modified_pos.y },
					{ SCast(float, TEXT_SPR_WIDTH) * scale.x, SCast(float, TEXT_SPR_HEIGHT) * scale.y },
					color,
					{ 0.0f,0.0f,0.0f },
					{ TEXT_SPR_WIDTH * SCast(float, SLASH_POS),0.0f },
					{ TEXT_SPR_WIDTH,TEXT_SPR_HEIGHT }
				);
				count++;
				break;
			default:
				break;
			}
		}
	}
}

// �����܂ސ��̕`��
void NumberRenderer::RenderFloat(float number, int decimal_points, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 scale, EnumNumberAlignment alignment, DirectX::XMFLOAT4 color, std::string command)
{
	int count	= 0;	// �`�悵�������̌�
	int adj_num = SCast(int, number) * PowInt(10, decimal_points);		// �����_���O��
	int num		= 0;	// �`�悷�鐔��
	int digits  = GetDigits(adj_num);

	DirectX::XMFLOAT2 modified_pos = {0.0f,0.0f}; // �A���C�����g������̈ʒu

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

	// �����`��
	if(command.empty())
	{
		while (digits > 0)
		{
			bool period = (decimal_points == digits) && (num != COLON_POS);

			// �����_��`�悷�邩�ۂ�
			if (period)	num = COLON_POS;
			else		num = adj_num / PowInt(10, digits - 1);

			number_sprite->Render(
				{ modified_pos.x + (TEXT_SPR_WIDTH * scale.x) * SCast(float,count),modified_pos.y },
				{ SCast(float, TEXT_SPR_WIDTH) * scale.x, SCast(float, TEXT_SPR_HEIGHT) * scale.y },
				color,
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
	else
	{
		for (int i = 0; i < command.size(); i++)
		{
			const char cmd = command.at(i);
			switch (cmd)
			{
			case 'N':
				break;
			default:
				break;
			}
		}
	}
}