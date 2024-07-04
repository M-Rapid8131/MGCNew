#ifndef __VIDEO_RENDERER_H__
#define __VIDEO_RENDERER_H__

// <>インクルード
#include <wrl.h>
#include <iostream>
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <Shlwapi.h>
#include <evr.h>

// ""インクルード
#include "misc.h"

// using
using Microsoft::WRL::ComPtr;

// enum
enum class EnumPlayerState
{
	CLOSED,
	READY,
	OPEN_PENDING,
	STARTED,
	PAUSED,
	STOPPED,
	CLOSING,
};

// class >> [VideoRenderer]
class VideoRenderer
{
private:
	// private:定数

	// private:構造体

public:
	// public:コンストラクタ・デストラクタ
	VideoRenderer(const wchar_t*);
	~VideoRenderer();

	// public:通常関数
	void				CreateSession();	
	void				CloseSession();
	void				StartPlayback();
	void				Play();
	void				Pause();
	void				Stop();
	void				Shutdown();
	void				Repaint();
	void				ResizeVideo(WORD width, WORD height);
	EnumPlayerState	GetState() const {}

	// public:ゲッター関数

	// public:セッター関数

private:
	// private:変数
	ComPtr<IMFMediaSession>			mf_media_session;
	ComPtr<IMFMediaSource>			mf_media_source;
	ComPtr<IMFVideoDisplayControl>	mf_video_display;
};

#endif // __VIDEO_RENDERER_H__