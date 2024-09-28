#ifndef __GAME_OBJECT_H__
#define __GAME_OBJECT_H__

// <>インクルード
#include <d3d11.h>
#include <memory>
#include <vector>
#include <magic_enum.hpp>

// ""インクルード
// LightBlueEngine
#include "3D/game_model.h"

// class >> [GameObject]
// 3Dモデルのクラスを内包したゲームに使用するためのクラス。
class GameObject
{
public:
	// public:コンストラクタ・デストラクタ
	GameObject() {}
	virtual ~GameObject() {}

	// public:通常関数
	virtual void	 UpdateTransform();

	// public:ゲッター関数
	const DirectX::XMFLOAT3&	GetTranslation() const	{ return translation; }
	const DirectX::XMFLOAT3&	GetAngle() const		{ return rotation; }
	const DirectX::XMFLOAT3&	GetScale() const		{ return scaling; }
	const DirectX::XMFLOAT4X4&	GetTransform() const		{ return transform; }
	virtual GameModel*			GetModel()					{ return model ? model.get() : nullptr; }

	// public:セッター関数
	void SetPosition(const DirectX::XMFLOAT3& new_position)	{ translation = new_position; }
	void SetAngle(const DirectX::XMFLOAT3& new_angle)		{ rotation = new_angle; }
	void SetScale(const DirectX::XMFLOAT3& new_scale)		{ scaling = new_scale; }

protected:

	// protected:変数
	bool						accumulate_particles	= true;
	bool						integrate_particles		= false;
	float						object_time				= 0.0f;
	DirectX::XMFLOAT3			translation				= { 0,0,0 };
	DirectX::XMFLOAT3			rotation				= { 0,0,0 };
	DirectX::XMFLOAT3			scaling					= { 1,1,1 };
	std::unique_ptr<GameModel>	model;

	DirectX::XMFLOAT4X4			transform = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};
};

#endif // __GAME_OBJECT_H__