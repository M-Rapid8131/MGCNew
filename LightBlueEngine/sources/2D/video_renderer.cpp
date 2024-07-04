// ""インクルード
// LightBlueEngine
#include "video_renderer.h"

// 関数
void CreateMediaSource(PCWSTR pcw_filename, IMFMediaSource** output_source)
{
	HRESULT hr = S_OK;

	MF_OBJECT_TYPE object_type = MF_OBJECT_INVALID;

	IMFSourceResolver*	mf_source_resolver	= nullptr;
	IUnknown*			source				= nullptr;

	MFCreateSourceResolver(&mf_source_resolver);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	
}

VideoRenderer::VideoRenderer(const wchar_t* w_filename)
{
	HRESULT hr = S_OK;

	// MediaFoundation関係の処理
	hr = CoInitialize(nullptr);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	IMFTopology*					mf_topology		= nullptr;
	IMFPresentationDescriptor*	mf_descriptor	= nullptr;

	CreateSession();

	
}

VideoRenderer::~VideoRenderer()
{

}