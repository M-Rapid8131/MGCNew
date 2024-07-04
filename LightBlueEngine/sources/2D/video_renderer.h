#ifndef __VIDEO_RENDERER_H__
#define __VIDEO_RENDERER_H__

// <>�C���N���[�h
#include <wrl.h>
#include <iostream>
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <Shlwapi.h>
#include <evr.h>

// ""�C���N���[�h
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
	// private:�萔

	// private:�\����

public:
	// public:�R���X�g���N�^�E�f�X�g���N�^
	VideoRenderer(const wchar_t*);
	~VideoRenderer();

	// public:�ʏ�֐�
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

	// public:�Q�b�^�[�֐�

	// public:�Z�b�^�[�֐�

private:
	// private:�ϐ�
	ComPtr<IMFMediaSession>			mf_media_session;
	ComPtr<IMFMediaSource>			mf_media_source;
	ComPtr<IMFVideoDisplayControl>	mf_video_display;
};

#endif // __VIDEO_RENDERER_H__