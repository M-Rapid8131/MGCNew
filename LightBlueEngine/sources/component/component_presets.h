#ifndef __COMPONENT_PRESETS_H__
#define __COMPONENT_PRESETS_H__

// ""インクルード
#include "component_base.h"

// class >> [Translate] 継承：Component<DirectX::XMFLOAT3>
// オブジェクトの移動を管理するコンポーネント。
class Translate : public Component<DirectX::XMFLOAT3>
{
public:
	// public:コンストラクタ・デストラクタ
	Translate(DirectX::XMFLOAT3 init_data) : Component<DirectX::XMFLOAT3>(init_data) {};
	~Translate() {};

	// public:通常関数
	void AddImpulse(DirectX::XMFLOAT3 impulse) 
	{
		DirectX::XMFLOAT3& f3_data = GetRef();
		f3_data.x += impulse.x;
		f3_data.y += impulse.y;
		f3_data.z += impulse.z;
	}

	// public:ゲッター関数
	const DirectX::XMFLOAT3&	Get()		{ return std::any_cast<const DirectX::XMFLOAT3&>(data); }
	DirectX::XMFLOAT3&			GetRef()	{ return std::any_cast<DirectX::XMFLOAT3&>(data); }

	// public:セッター関数
	void Set(const DirectX::XMFLOAT3& new_data) { Component::Set<DirectX::XMFLOAT3>(new_data); }
};

// class >> [Scale] 継承：Component<DirectX::XMFLOAT3>
// オブジェクトのサイズを管理するコンポーネント。
class Scale : public Component<DirectX::XMFLOAT3>
{
public:
	// public:コンストラクタ・デストラクタ
	Scale(DirectX::XMFLOAT3 init_data) : Component<DirectX::XMFLOAT3>(init_data) {};
	~Scale() {};

	// public:ゲッター関数
	const DirectX::XMFLOAT3&	Get()		{ return std::any_cast<const DirectX::XMFLOAT3&>(data); }
	DirectX::XMFLOAT3&			GetRef()	{ return std::any_cast<DirectX::XMFLOAT3&>(data); }

	// public:セッター関数
	void Set(const DirectX::XMFLOAT3& new_data) { Component::Set<DirectX::XMFLOAT3>(new_data); }
};

#endif // __COMPONENT_PRESETS_H__

