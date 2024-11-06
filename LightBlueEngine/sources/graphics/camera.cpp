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

	D3D11_BUFFER_DESC buffer_desc = {};
	buffer_desc.ByteWidth			= sizeof(CbScene);
	buffer_desc.Usage				= D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags		= 0;
	buffer_desc.MiscFlags			= 0;
	buffer_desc.StructureByteStride = 0;
	device->CreateBuffer(&buffer_desc, nullptr, scene_cbuffer.GetAddressOf());

	DirectX::XMFLOAT3 angle = {0.0f, 0.0f, 0.0f };
	switch (camera_mode)
	{
	case EnumCameraMode::FIRST_PERSON_VIEW:
		angle = AddFPVCamera().fpv_angle;
		break;
	case EnumCameraMode::THIRD_PERSON_VIEW:
		angle = AddTPVCamera().tpv_angle;
		break;
	default:
		break;
	}

	// ビュー行列の作成
	DirectX::XMVECTOR v_eye		= DirectX::XMLoadFloat3(&camera_position);
	DirectX::XMVECTOR v_focus	= DirectX::XMLoadFloat3(&camera_focus);

	DirectX::XMVECTOR v_front	= DirectX::XMVectorSubtract(v_focus, v_eye);
	DirectX::XMVECTOR v_up		= DirectX::XMVectorSet(0, -1, 0, 0);

	DirectX::XMVECTOR nv_front	= DirectX::XMVector3Normalize(v_front);
	DirectX::XMVECTOR nv_right	= DirectX::XMVector3Normalize(DirectX::XMVector3Cross(nv_front, v_up));
	v_up = DirectX::XMVector3Cross(nv_front, nv_right);

	DirectX::XMMATRIX m_view = DirectX::XMMatrixLookAtLH(v_eye, v_focus, v_up);

	DirectX::XMStoreFloat4x4(&view, m_view);

	// ビューの逆行列を、ワールド行列へ
	DirectX::XMMATRIX		m_world = DirectX::XMMatrixInverse(nullptr, m_view);
	DirectX::XMFLOAT4X4		world;
	DirectX::XMStoreFloat4x4(&world, m_world);

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
			// 一人称カメラ取得
			FPVData& fpv	= fpv_data[fpv_channel];

			// 角度をラジアンに変換
			DirectX::XMFLOAT3 rad_angle = {};
			rad_angle.x = DirectX::XMConvertToRadians(fpv.fpv_angle.x);
			rad_angle.y = DirectX::XMConvertToRadians(fpv.fpv_angle.y);
			rad_angle.z = DirectX::XMConvertToRadians(fpv.lock_z_rotate ? 0.0f : fpv.fpv_angle.z);

			// ラジアン角から回転行列を作成
			DirectX::XMMATRIX m_rotation = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&rad_angle));
			DirectX::XMFLOAT4X4 rotation_matrix;
			DirectX::XMStoreFloat4x4(&rotation_matrix, m_rotation);

			// 右、上、前のベクトルを取得
			DirectX::XMVECTOR v_right	= { rotation_matrix._11,rotation_matrix._12,rotation_matrix._13 };
			DirectX::XMVECTOR v_up		= { rotation_matrix._21,rotation_matrix._22,rotation_matrix._23 };
			DirectX::XMVECTOR v_forward = { rotation_matrix._31,rotation_matrix._32,rotation_matrix._33 };

			// ３つのベクトルを念のため正規化、上方向だけ反転させておく
			v_right		= DirectX::XMVector3Normalize(v_right);
			v_up		= DirectX::XMVector3Normalize(DirectX::XMVectorScale(v_up, -1));
			v_forward	= DirectX::XMVector3Normalize(v_forward);

			// 右方向ベクトルと上方向ベクトルを調整
			v_right	= DirectX::XMVector3Normalize(DirectX::XMVector3Cross(v_forward, v_up));
			v_up	= DirectX::XMVector3Cross(v_forward, v_right);

			// カメラの注視点、位置を設定
			camera_position = fpv.fpv_position;
			DirectX::XMStoreFloat3(&camera_focus, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&camera_position), v_forward));

			// カメラの移動を反映させた位置、注視点を作成
			DirectX::XMFLOAT3 moved_camera_position = XMFloatCalclation::XMFloat3Add(camera_position, camera_shift);
			DirectX::XMFLOAT3 moved_camera_focus	= XMFloatCalclation::XMFloat3Add(camera_focus, camera_shift);

			// 位置、注視点をベクトルに格納
			DirectX::XMVECTOR v_eye		= DirectX::XMLoadFloat3(&moved_camera_position);
			DirectX::XMVECTOR v_focus	= DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&moved_camera_focus), v_forward);

			// ビュー行列を作成
			DirectX::XMMATRIX m_view	= DirectX::XMMatrixLookAtLH(v_eye, v_focus, v_up);
			DirectX::XMStoreFloat4x4(&view, m_view);

			// ビューの逆行列を、ワールド行列へ
			DirectX::XMMATRIX		m_world = DirectX::XMMatrixInverse(nullptr, m_view);
			DirectX::XMFLOAT4X4		world;
			DirectX::XMStoreFloat4x4(&world, m_world);

			// 視野角、画面比率、クロップ距離からプロジェクション行列を作成
			float screen_width	= SCast(float, graphics->GetScreenWidth());
			float screen_height = SCast(float, graphics->GetScreenHeight());

			// プロジェクション行列を作成
			DirectX::XMMATRIX m_projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov), screen_width / screen_height, near_z, far_z);
			DirectX::XMStoreFloat4x4(&projection, m_projection);

			// 定数バッファにそれぞれの値を入れる
			scene_constants.view			= view;
			scene_constants.projection		= projection;
			scene_constants.camera_position = XMFloatCalclation::XMFloat3Add(camera_position, camera_shift);
		}
	}

	else if (camera_mode == EnumCameraMode::THIRD_PERSON_VIEW)
	{
		if (!tpv_data.empty())
		{
			// 三人称カメラ取得
			TPVData& tpv	= tpv_data[tpv_channel];

			// 角度をラジアンに変換
			DirectX::XMFLOAT3 rad_angle = {};
			rad_angle.x = DirectX::XMConvertToRadians(tpv.tpv_angle.x);
			rad_angle.y = DirectX::XMConvertToRadians(tpv.tpv_angle.y);
			rad_angle.z = DirectX::XMConvertToRadians(tpv.lock_z_rotate ? 0.0f : tpv.tpv_angle.z);

			// ラジアン角から回転行列を作成
			DirectX::XMMATRIX m_rotation = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&rad_angle));
			DirectX::XMFLOAT4X4 rotation_matrix;
			DirectX::XMStoreFloat4x4(&rotation_matrix, m_rotation);

			// 右、上、前のベクトルを取得
			DirectX::XMVECTOR v_right	= { rotation_matrix._11,rotation_matrix._12,rotation_matrix._13 };
			DirectX::XMVECTOR v_up		= { rotation_matrix._21,rotation_matrix._22,rotation_matrix._23 };
			DirectX::XMVECTOR v_forward = { rotation_matrix._31,rotation_matrix._32,rotation_matrix._33 };

			// ３つのベクトルを念のため正規化、上方向だけ反転させておく
			v_right		= DirectX::XMVector3Normalize(v_right);
			v_up		= DirectX::XMVector3Normalize(DirectX::XMVectorScale(v_up, -1));
			v_forward	= DirectX::XMVector3Normalize(v_forward);

			// 右方向ベクトルと上方向ベクトルを調整
			v_right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(v_forward, v_up));
			v_up	= DirectX::XMVector3Cross(v_forward, v_right);

			// カメラの注視点、位置を設定
			camera_focus = tpv.tpv_target;
			DirectX::XMStoreFloat3(&camera_position, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&tpv.tpv_target), DirectX::XMVectorScale(v_forward, tpv.tpv_distance)));

			// カメラの移動を反映させた位置、注視点を作成
			DirectX::XMFLOAT3 moved_camera_position = XMFloatCalclation::XMFloat3Add(camera_position, camera_shift);
			DirectX::XMFLOAT3 moved_camera_focus	= XMFloatCalclation::XMFloat3Add(camera_focus, camera_shift);

			// 位置、注視点をベクトルに格納
			DirectX::XMVECTOR v_eye		= DirectX::XMLoadFloat3(&moved_camera_position);
			DirectX::XMVECTOR v_focus	= DirectX::XMLoadFloat3(&moved_camera_focus);

			// ビュー行列を作成
			DirectX::XMMATRIX m_view = DirectX::XMMatrixLookAtLH(v_eye, v_focus, v_up);
			DirectX::XMStoreFloat4x4(&view, m_view);

			// ビューの逆行列を、ワールド行列へ
			DirectX::XMMATRIX		m_world = DirectX::XMMatrixInverse(nullptr, m_view);
			DirectX::XMFLOAT4X4		world;
			DirectX::XMStoreFloat4x4(&world, m_world);

			// 視野角、画面比率、クロップ距離からプロジェクション行列を作成
			float screen_width = SCast(float, graphics->GetScreenWidth());
			float screen_height = SCast(float, graphics->GetScreenHeight());

			// プロジェクション行列を作成
			DirectX::XMMATRIX m_projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov), screen_width / screen_height, near_z, far_z);
			DirectX::XMStoreFloat4x4(&projection, m_projection);

			// 定数バッファにそれぞれの値を入れる
			scene_constants.view		= view;
			scene_constants.projection	= projection;
			scene_constants.camera_position = XMFloatCalclation::XMFloat3Add(camera_position, camera_shift);
		}
	}

	camera_shift = XMFloatCalclation::XMFloat3Lerp(camera_shift, { 0.0f,0.0f,0.0f }, 0.1f);
	scene_constants.blur_size			= std::lerp(scene_constants.blur_size, 0.0f, BLUR_ATTENUATION);
	scene_constants.post_effect_blend	= std::lerp(scene_constants.post_effect_blend, 1.0f, BLUR_ATTENUATION);

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

		ImGui::InputFloat3("CameraShift", &camera_shift.x);

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
			ImGui::Checkbox("Lock Z Angle", &fpv.lock_z_rotate);
			ImGui::DragFloat3("Camera Position", &fpv.fpv_position.x, 0.05f);
			ImGui::DragFloat3("Camera Angle", &fpv.fpv_angle.x, 0.05f);

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
			ImGui::Checkbox("Lock Z Angle", &tpv.lock_z_rotate);
			ImGui::DragFloat3("Camera Target", &tpv.tpv_target.x, 0.01f);
			ImGui::DragFloat3("Camera Angle", &tpv.tpv_angle.x, 0.05f);
			ImGui::DragFloat("Camera Distance", &tpv.tpv_distance, 0.01f, 0.01f, 1000.0f);

			// カメラ追加
			if (ImGui::Button("Add TPVCamera"))
				AddTPVCamera();
		}
	}
}

// 一人称視点カメラ追加
Camera::FPVData& Camera::AddFPVCamera(FPVData* fpv_init)
{
	FPVData& fpv = fpv_data.emplace_back();
	fpv.fpv_position	= fpv_init ? fpv_init->fpv_position		: DEFAULT_FPV_POSITION;
	fpv.fpv_angle	= fpv_init ? fpv_init->fpv_angle	: DEFAULT_FPV_ANGLE;
	return fpv;
}

// 三人称視点カメラ追加
Camera::TPVData& Camera::AddTPVCamera(TPVData* tpv_init)
{
	TPVData& tpv = tpv_data.emplace_back();
	tpv.tpv_target		= tpv_init ? tpv_init->tpv_target		: DEFAULT_TPV_TARGET;
	tpv.tpv_angle	= tpv_init ? tpv_init->tpv_angle	: DEFAULT_TPV_ANGLE;
	tpv.tpv_distance	= tpv_init ? tpv_init->tpv_distance		: DEFAULT_TPV_DISTANCE;
	return tpv;
}