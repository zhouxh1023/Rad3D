/*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "stdafx.h"
#include "DebugInfo.h"

class App : public Singleton<App>
{
public:
    App();
    virtual ~App();

	virtual void Init(HINSTANCE hInst, HWND hWnd, int w, int h);
    virtual void Update();
	virtual void Shutdown();
	virtual void Pause();
	virtual void Resume();
	virtual void Resize(int w, int h);

	virtual void InjectMouseMove(float _x, float _y);
	virtual void InjectMouseWheel(float _z);
	virtual void InjectMouseDown(int _id, float _x, float _y);
	virtual void InjectMouseUp(int _id, float _x, float _y);
	virtual void InjectKeyDown(int _key, uchar_t _text);
	virtual void InjectKeyUp(int _key);

	virtual void InjectTouchDown(int _id, float _x, float _y);
	virtual void InjectTouchUp(int _id, float _x, float _y);
	virtual void InjectTouchMove(int _id, float _x, float _y);
	virtual void InjectTouchCancel(int _id, float _x, float _y);

	virtual void OnSetupResource() = 0;
	virtual void OnInit() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnShutdown() = 0;
	virtual void OnPause() {}
	virtual void OnResume() {}
	virtual void OnResize(int w, int h) {}
	virtual void OnDragFile(const char * filename) {}

	void MapScreenUnit(float & x, float & y);

	HINSTANCE _hInst() { return mInst; }
	HWND _hWnd() { return mWnd; }

protected:
#ifdef M_PLATFORM_WIN32
	HINSTANCE mInst;
	HWND mWnd;
#endif

	Root * mRoot;
	ResourceManager * mResourceManager;
	GLRenderSystem * mRenderSystem;
	ALAudioSystem * mAudioSystem;
	MGUI::Engine * mUIEngine;
	ParticleFX * mParticleFX;
	PhyWorld * mPhyWorld;
	World * mWorld;
	DebugInfo * mDebugInfo;

	DIInputSystem * mInputSystem;

	bool mPause;
	float mFPSLimit;
	float mInternalLastTime;
};

#ifdef M_PLATFORM_WIN32
#ifdef APP_EXPORT
#define APP_ENTRY __declspec(dllexport)
#endif
#endif

#ifndef APP_ENTRY
#define APP_ENTRY
#endif

extern "C" {

	APP_ENTRY void CreateApplication(App ** ppApp);

	typedef void (*CREATE_APPLICATION_FUNCTION)(App ** ppApp);
};


