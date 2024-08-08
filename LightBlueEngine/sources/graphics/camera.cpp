// <>インクルード
#include <imgui.h>

// ""インクルード
// LightBlueEngine
#include "camera.h"
#include "graphics.h"
#include "xmfloat_calclation.h"

//-----------------------------------------
// Camera メンバ関数
//-----------------------------------------

// コンストラクタ
Camera::Camera(EnumCameraMode& start_mode) : camera_mode(start_mode)
{
	Graphics*		graphics	= Graphics::GetInstance();
	ID3D11Device*	device		= graphics->GetDevice().Get();

	// scale、rotation、translationに初期値を設定
	DirectX::XMStoreFloat4x4(&scale,		DirectX::XMMatrixScaling(1, 1, 1));
	DirectX::XMStoreFloat4x4(&rotation,		DirectX::XMMatrixRotationRollPitchYaw(0, 0, 0));
	DirectX::XMStoreFloat4x4(&translation,	DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&scene_constants.camera_position)));

	D3D11_BUFFER_DESC buffer_desc = {};
	buffer_desc.ByteWidth			= sizeof(CbScene);
	buffer_desc.Usage				= D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags		= 0;
	buffer_desc.MiscFlags			= 0;
	buffer_desc.StructureByteStride = 0;
	device->CreateBuffer(&buffer_desc, nullptr, scene_cbuffer.GetAddressOf());

	DirectX::XMFLOAT4 direction = {0.0f, 0.0f, 1.0f, 0.0f};
	switch (camera_mode)
	{
	case EnumCameraMode::FIRST_PERSON_VIEW:
		if (!fpv_data.empty())
			direction = fpv_data[fpv_channel].fpv_direction;
		break;
	case EnumCameraMode::THIRD_PERSON_VIEW:
		if (!tpv_data.empty())
			direction = tpv_data[tpv_channel].tpv_direction;
		break;
	default:
		break;
	}

	// ビュー行列の作成
	DirectX::XMVECTOR v_eye		= DirectX::XMLoadFloat3(&camera_position);
	DirectX::XMVECTOR v_focus	= DirectX::XMLoadFloat3(&camera_focus);
	DirectX::XMVECTOR v_front	= DirectX::XMVectorSubtract(v_focus, v_eye);
	DirectX::XMVECTOR v_up		= DirectX::XMVectorAdd(v_eye, DirectX::XMVectorSet(0, -1, 0, 0));

	DirectX::XMVECTOR nv_front	= DirectX::XMVector3Normalize(v_front);
	DirectX::XMVECTOR nv_right	= DirectX::XMVector3Normalize(DirectX::XMVector3Cross(nv_front, v_up));
	v_up = DirectX::XMVector3Cross(nv_front, nv_right);

	DirectX::XMMATRIX m_view = DirectX::XMMatrixLookAtLH(v_eye, v_focus, v_up);

	DirectX::XMStoreFloat4x4(&view, m_view);

	// ビューの逆行列を、ワールド行列へ
	DirectX::XMMATRIX		m_world = DirectX::XMMatrixInverse(nullptr, m_view);
	DirectX::XMFLOAT4X4		world;
	DirectX::XMStoreFloat4x4(&world, m_world);

	// カメラの方向を取り出す
	right	= { world._11,world._12,world._13 };
	up		= { world._21,world._22,world._23 };
	front	= { world._31,world._32,world._33 };

	// 視野角、画面比率、クロップ距離からプロジェクション行列を作成
	float screen_width	= SCast(float, graphics->GetScreenWidth());
	float screen_height = SCast(float, graphics->GetScreenHeight());

	DirectX::XMMATRIX m_projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov), screen_width / screen_height, near_z, far_z);
	DirectX::XMStoreFloat4x4(&projection, m_projection);
}

// 更新処理
void Camera::Update()
{
	Graphics* graphics = Graphics::GetInstance();
	ID3D11DeviceContext* device_context = graphics->GetDeviceContext().Get();

	if (camera_mode == EnumCameraMode::FIRST_PERSON_VIEW)
	{
		if (!fpv_data.empty())
		{
			FPVData& fpv	= fpv_data[fpv_channel];
			DirectX::XMFLOAT3 direction = {
				fpv.fpv_direction.x,
				fpv.fpv_direction.y,
				fpv.fpv_direction.z
			};

			camera_position = fpv.fpv_position;
			camera_focus	= XMFloat3Add(fpv.fpv_position, direction);

			DirectX::XMVECTOR v_direction = DirectX::XMLoadFloat4(&fpv.fpv_direction);
			v_direction = DirectX::XMVector4Normalize(v_direction);
			DirectX::XMStoreFloat4(&fpv.fpv_direction, v_direction);
		}
	}

	else if (camera_mode == EnumCameraMode::THIRD_PERSON_VIEW)
	{
		if (!tpv_data.empty())
		{
			TPVData& tpv	= tpv_data[tpv_channel];
			DirectX::XMFLOAT3 direction = {
				tpv.tpv_direction.x,
				tpv.tpv_direction.y,
				tpv.tpv_direction.z
			};

			camera_focus	= tpv.tpv_target;
			camera_position = XMFloat3Add(tpv.tpv_target, XMFloat3Scale(direction, tpv.tpv_distance));
		
			DirectX::XMVECTOR v_direction = DirectX::XMLoadFloat4(&tpv.tpv_direction);
			v_direction = DirectX::XMVector4Normalize(v_direction);
			DirectX::XMStoreFloat4(&tpv.tpv_direction, v_direction);
		}
	}

	DirectX::XMFLOAT3 moved_camera_position = XMFloatCalclation::XMFloat3Add(camera_position, camera_shift);
	DirectX::XMFLOAT3 moved_camera_focus	= XMFloatCalclation::XMFloat3Add(camera_focus, camera_shift);

	camera_shift = XMFloatCalclation::XMFloat3Lerp(camera_shift, { 0.0f,0.0f,0.0f }, 0.1f);
	scene_constants.blur_size			= std::lerp(scene_constants.blur_size, 0.0f, BLUR_ATTENUATION);
	scene_constants.post_effect_blend	= std::lerp(scene_constants.post_effect_blend, 1.0f, BLUR_ATTENUATION);

	// ビュー行列の作成
	DirectX::XMFLOAT4 direction = { 0.0f, 0.0f, 1.0f, 0.0f };
	switch (camera_mode)
	{
	case EnumCameraMode::FIRST_PERSON_VIEW:
		if (!fpv_data.empty())
			direction = fpv_data[fpv_channel].fpv_direction;
		break;
	case EnumCameraMode::THIRD_PERSON_VIEW:
		if (!tpv_data.empty())
			direction = tpv_data[tpv_channel].tpv_direction;
		break;
	default:
		break;
	}

	DirectX::XMVECTOR v_eye		= DirectX::XMLoadFloat3(&moved_camera_position);
	DirectX::XMVECTOR v_focus	= DirectX::XMLoadFloat3(&moved_camera_focus);
	DirectX::XMVECTOR nv_front	= DirectX::XMLoadFloat4(&direction);
	DirectX::XMVECTOR v_up		= DirectX::XMVectorSet(0, -1, 0, 0);

	DirectX::XMVECTOR nv_right	= DirectX::XMVector3Normalize(DirectX::XMVector3Cross(nv_front, v_up));
	v_up = DirectX::XMVector3Cross(nv_front, nv_right);

	DirectX::XMMATRIX m_view	= DirectX::XMMatrixLookAtLH(v_eye, v_focus, v_up);

	DirectX::XMStoreFloat4x4(&view, m_view);

	// ビューの逆行列を、ワールド行列へ
	DirectX::XMMATRIX		m_world = DirectX::XMMatrixInverse(nullptr, m_view);
	DirectX::XMFLOAT4X4		world;
	DirectX::XMStoreFloat4x4(&world, m_world);

	// カメラの方向を取り出す
	right	= { world._11,world._12,world._13 };
	up		= { world._21,world._22,world._23 };
	front	= { world._31,world._32,world._33 };

	// 視野角、画面比率、クロップ距離からプロジェクション行列を作成
	float screen_width	= SCast(float, graphics->GetScreenWidth());
	float screen_height = SCast(float, graphics->GetScreenHeight());

	DirectX::XMMATRIX m_projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov), screen_width / screen_height, near_z, far_z);
	DirectX::XMStoreFloat4x4(&projection, m_projection);

	scene_constants.view			= view;
	scene_constants.projection		= projection;
	scene_constants.camera_position = moved_camera_position;

	device_context->UpdateSubresource(scene_cbuffer.Get(), 0, 0, &scene_constants, 0, 0);

	// b10にセット
	device_context->VSSetConstantBuffers(
		SCast(UINT, EnumCommonConstantBuffer::SCENE), 1, scene_cbuffer.GetAddressOf());
	device_context->PSSetConstantBuffers(
		SCast(UINT, EnumCommonConstantBuffer::SCENE), 1, scene_cbuffer.GetAddressOf());
	device_context->CSSetConstantBuffers(
		SCast(UINT, EnumCommonConstantBuffer::SCENE), 1, scene_cbuffer.GetAddressOf());
	device_context->GSSetConstantBuffers(
		SCast(UINT, EnumCommonConstantBuffer::SCENE), 1, scene_cbuffer.GetAddressOf());
}

// ImGui表示
void Camera::DebugGUI()
{
	if (ImGui::CollapsingHeader("Camera"))
	{
		// カメラの視野角などの設定
		ImGui::DragFloat("FoV", &fov, 0.01f, 0.1f, 180.0f);
		ImGui::DragFloat("Near Z", &near_z, 0.01f, 0.01f, far_z);
		ImGui::DragFloat("Far Z", &far_z, 0.01f, near_z, 5000.0f);

		// カメラのベクトルの確認
		ImGui::InputFloat3("Camera Up", &up.x);
		ImGui::InputFloat3("Camera Front", &front.x);
		ImGui::InputFloat3("Camera Right", &right.x);

		// カメラモード切り替え
		const char* modes[] = {
			"FirstPersonView",
			"ThirdPersonView",
		};

		int selecting_id = SCast(int, camera_mode);
		const char* selecting_mode = modes[selecting_id];

		if (ImGui::BeginCombo("Camera Mode", selecting_mode))
		{
			for (int i = 0; i < SCast(int, EnumCameraMode::CAMERA_MODE_NUM); i++)
			{
				const bool is_selected = (selecting_id == i);
				if (ImGui::Selectable(modes[i], is_selected))
				{
					selecting_id = i;

					switch (SCast(EnumCameraMode, i))
					{
					case EnumCameraMode::FIRST_PERSON_VIEW:
						if (fpv_data.empty())
							AddFPVCamera();
						break;
					case EnumCameraMode::THIRD_PERSON_VIEW:
						if (tpv_data.empty())
							AddTPVCamera();
						break;
					default:
						break;
					}
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();

				camera_mode = SCast(EnumCameraMode, selecting_id);
			}
			ImGui::EndCombo();
		}

		// 一人称視点カメラ
		if (camera_mode == EnumCameraMode::FIRST_PERSON_VIEW)
		{
			size_t min_channel = 0;
			size_t max_channel = fpv_data.size() - 1;

			FPVData& fpv = fpv_data[fpv_channel];

			ImGui::SliderScalar("FPV Channel", ImGuiDataType_U64, &fpv_channel, &min_channel, &max_channel);
			ImGui::DragFloat3("Camera Position", &fpv.fpv_position.x, 0.05f);
			ImGui::DragFloat4("Camera Direction", &fpv.fpv_direction.x, 0.05f);

			// カメラ追加
			if (ImGui::Button("Add FPVCamera"))
				AddFPVCamera();
		}

		// 三人称視点カメラ
		else if (camera_mode == EnumCameraMode::THIRD_PERSON_VIEW)
		{
			size_t min_channel = 0;
			size_t max_channel = tpv_data.size() - 1;

			TPVData& tpv = tpv_data[tpv_channel];

			ImGui::SliderScalar("FPV Channel", ImGuiDataType_U64, &tpv_channel, &min_channel, &max_channel);
			ImGui::DragFloat3("Camera Target", &tpv.tpv_target.x, 0.05f);
			ImGui::DragFloat4("Camera Direction", &tpv.tpv_direction.x, 0.05f);
			ImGui::DragFloat("Camera Distance", &tpv.tpv_distance, 0.01f, 0.01f, 1000.0f);

			// カメラ追加
			if (ImGui::Button("Add TPVCamera"))
				AddTPVCamera();
		}
	}
}

// 一人称視点カメラ追加
void Camera::AddFPVCamera(FPVData* fpv_init)
{
	FPVData& fpv = fpv_data.emplace_back();
	fpv.fpv_position	= fpv_init ? fpv_init->fpv_position		: DEFAULT_FPV_POSITION;
	fpv.fpv_direction	= fpv_init ? fpv_init->fpv_direction	: DEFAULT_FPV_DIRECTION;
}

// 三人称視点カメラ追加
void Camera::AddTPVCamera(TPVData* tpv_init)
{
	TPVData& tpv = tpv_data.emplace_back();
	tpv.tpv_target		= tpv_init ? tpv_init->tpv_target		: DEFAULT_TPV_TARGET;
	tpv.tpv_direction	= tpv_init ? tpv_init->tpv_direction	: DEFAULT_TPV_DIRECTION;
	tpv.tpv_distance	= tpv_init ? tpv_init->tpv_distance		: DEFAULT_TPV_DISTANCE;
}