// ""インクルード
// LightBlueEngine
#include "graphics/graphics.h"
#include "engine_utility.h"
#include "imgui_renderer.h"
#include "implot.h"

//-------------------------------------------
// ImGuiRenderer メンバ関数
//-------------------------------------------

// コンストラクタ
ImGuiRenderer::ImGuiRenderer(HWND hwnd)
{
	Graphics*				graphics		= Graphics::GetInstance();
	ID3D11Device*			device			= graphics->GetDevice().Get();
	ID3D11DeviceContext*	device_context	= graphics->GetDeviceContext().Get();

	// ImGui初期設定
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("resources/fonts/MPLUS2-Medium.ttf", 20.0f, nullptr, glyphRangesJapanese);
	//io.ConfigFlags	|= ImGuiConfigFlags_DockingEnable;   // Window同士をくっつける
	//io.ConfigFlags	|= ImGuiConfigFlags_ViewportsEnable;
	io.BackendFlags		|= ImGuiBackendFlags_PlatformHasViewports;
	io.BackendFlags		|= ImGuiBackendFlags_RendererHasViewports;

	ImGui_ImplWin32_Init(RCast(void*, hwnd));
	ImGui_ImplDX11_Init(device, device_context);
	ImGui::StyleColorsLight();
	ImGui::PushStyleColor(ImGuiCol_TitleBg,				ImVec4(NormC(80.0f), NormC(200.0f), NormC(255.0f), 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive,		ImVec4(NormC(230.0f), NormC(190.0f), NormC(255.0f), 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed,	ImVec4(NormC(140.0f), NormC(225.0f), NormC(255.0f), 0.3f));
}

// デストラクタ
ImGuiRenderer::~ImGuiRenderer()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
}

// ImGuiのNewFrame
void ImGuiRenderer::Begin()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

// 描画処理
void ImGuiRenderer::Render()
{
	ImGui::Render();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}