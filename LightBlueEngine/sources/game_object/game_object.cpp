// ""インクルード
// LightBlueEngine
#include "game_object.h"

//--------------------------------------
// GameObject メンバ関数
//--------------------------------------

// transform行列更新
void GameObject::UpdateTransform()
{
	// 大きさ
	DirectX::XMMATRIX m_scaling = DirectX::XMMatrixScaling(scaling.x, scaling.y, scaling.z);

	// 回転
	DirectX::XMMATRIX m_rotation_x = DirectX::XMMatrixRotationX(rotation.x);
	DirectX::XMMATRIX m_rotation_y = DirectX::XMMatrixRotationY(rotation.y);

	DirectX::XMMATRIX m_rotation_z = DirectX::XMMatrixRotationZ(rotation.z);
	DirectX::XMMATRIX m_rotation = m_rotation_y * m_rotation_x * m_rotation_z;

	// 移動値
	DirectX::XMMATRIX m_translation = DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);

	DirectX::XMMATRIX m_transform = m_scaling * m_rotation * m_translation;

	DirectX::XMStoreFloat4x4(&transform, m_transform);
}