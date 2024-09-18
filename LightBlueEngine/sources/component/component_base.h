#ifndef __COMPONENT_BASE_H__
#define __COMPONENT_BASE_H__

// <>インクルード
#include <vector>
#include <any>
#include <DirectXMath.h>

// class >> [Component]
// コンポーネントの元となるクラス。
template<class Type>
class Component
{
public:
	// public:コンストラクタ・デストラクタ
	template<class Type> Component(Type init_data) : data(init_data) = 0;
	virtual ~Component() {};

	// public:ゲッター関数
	template<class Type> virtual const Type& Get() = 0;
	template<class Type> virtual Type& GetRef() = 0;

	// public:セッター関数
	template<class Type> virtual void Set(const Type& new_data) { data = new_data; };

protected:
	std::any data;
};

#endif // __COMPONENT_BASE_H__

