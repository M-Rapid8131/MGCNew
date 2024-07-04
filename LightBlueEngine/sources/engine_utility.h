#ifndef __ENGINE_UTILITY_H__
#define __ENGINE_UTILITY_H__

// <>インクルード
#include <string>
#include <DirectXMath.h>
#include <magic_enum.hpp>
#include <any>

// マクロ
// static_cast短縮
#define SCast(type,num)		static_cast<type>(num)

// const_cast短縮
#define CCast(type,num)		const_cast<type>(num)

// reinterpret_cast短縮
#define RCast(type,num)		reinterpret_cast<type>(num)

// any_cast短縮
#define ACast(type,num)		std::any_cast<type>(num)

// 計算シェーダー用
#define Align(num, alignment) (num + (alignment - 1) & ~(alignment - 1))

// 0~255を0.0~1.0に変換
#define NormC(value_255)	value_255 / 255.0f

// 構造体
// struct >> [FlagSystem]
template<typename Enum>
struct FlagSystem
{
	// 関数
	bool GetFlag(Enum item)				// フラグが立っているかを確認
	{
		int item_num = magic_enum::enum_integer<Enum>(item);
		return flags & (1 << item_num);
	}

	void	 SetFlag(Enum item, bool flg)	// フラグを設定する
	{
		int item_num = magic_enum::enum_integer<Enum>(item);
		flg ? flags |= (1 << item_num) : flags &= ~(1 << item_num);
	}

	void	 SetAllFlag(unsigned int flag)
	{
		flags = flag;
	}

	// 変数
	unsigned int		flags = 0;
};

#endif // __ENGINE_UTILITY_H__