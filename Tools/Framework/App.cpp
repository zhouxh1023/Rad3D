#include "App.h"

ImplementSingleton(App);

App::App()
{
	mRoot = NULL;
	mResourceManager = NULL;
	mRenderSystem = NULL;
	mAudioSystem = NULL;
	mUIEngine = NULL;
	mParticleFX = NULL;
	mWorld = NULL;
	mPhyWorld = NULL;

	mDebugInfo = NULL;

	mPause = false;
	mFPSLimit = 1.0f / 60.0f;
	mInternalLastTime = 0;
}

App::~App()
{
}

void App::Init(HINSTANCE hInst, HWND hWnd, int w, int h)
{
#ifdef M_PLATFORM_WIN32
	mInst = hInst;
	mWnd = hWnd;
#endif

	mRoot = new Root;

	mResourceManager = new ResourceManager;
	OnSetupResource();

	RenderSystem::Config config;
	config.width = w;
	config.height = h;
	mRenderSystem = new GLRenderSystem(hWnd, config);

	mInputSystem = new DIInputSystem(hInst, hWnd);

	mAudioSystem = new ALAudioSystem;

	mUIEngine = new MGUI::Engine;

	mParticleFX = new ParticleFX;

	mPhyWorld = new PhyWorld;

	mWorld = new World;

	OnInit();

	mDebugInfo = new DebugInfo;

	mInternalLastTime = mRoot->GetTime();

	DragAcceptFiles(hWnd, TRUE);
}

void App::Update()
{
	mRoot->Update();

	mInputSystem->Update();

	if (!mPause)
	{
		OnUpdate();

		float elapsedTime = mRoot->GetFrameTime();

		if (elapsedTime > 0.1f)
			elapsedTime = 0.1f;

		mPhyWorld->Update(elapsedTime);

		mWorld->Update(elapsedTime);

		mUIEngine->Update(elapsedTime);

		mDebugInfo->Update();

		mRenderSystem->Begin();

		mWorld->Render();

		mUIEngine->Render();

		mRenderSystem->End();

		mAudioSystem->Update(World::Instance()->MainCamera()->GetPosition());
	}
	
	// FPS Limit
	float dTime = (mRoot->GetTime() - mInternalLastTime);
	if (dTime < mFPSLimit)
	{
		DWORD sleepTime = (DWORD)((mFPSLimit - dTime) * 1000.0f);
		Thread::Sleep(sleepTime);
	}

	mInternalLastTime = mRoot->GetTime();
}

void App::Shutdown()
{
	d_printf("-: Game Shutdown...");
	OnShutdown();

	d_printf("-: Engine Shutdown...");
	delete mDebugInfo;
	delete mWorld;
	delete mPhyWorld;
	delete mParticleFX;
	delete mUIEngine;
	delete mAudioSystem;
	delete mInputSystem;
	delete mRenderSystem;
	delete mResourceManager;
	delete mRoot;

	d_printf("-: Shutdown OK...");
}

void App::Pause()
{
	if (!mPause)
	{
		OnPause();

		d_printf("-: Lost device...");

		mRenderSystem->OnLostDevice();

		mPause = true;
	}
}

void App::Resume()
{
	if (mPause)
	{
		d_printf("-: Reset device...");

		mRenderSystem->OnResetDevice();

		OnResume();

		mPause = false;
	}
}

void App::Resize(int w, int h)
{
	if (mRoot)
	{
		mUIEngine->OnResize(w, h);
		mRenderSystem->OnResize(w, h);


		OnResize(w, h);
	}
}

void App::MapScreenUnit(float & x, float & y)
{
	Float2 sz = MGUI::Engine::Instance()->GetInvSize();

	x *= sz.x;
	y *= sz.y;
}

void App::InjectMouseMove(float _x, float _y)
{
	MGUI::InputManager::Instance()->_injectMouseMove(_x, _y);
}

void App::InjectMouseWheel(float _z)
{
	MGUI::InputManager::Instance()->_injectMouseWheel(_z);
}

void App::InjectMouseDown(int _id, float _x, float _y)
{
	MGUI::InputManager::Instance()->_injectMouseDown(_id, _x, _y);
}

void App::InjectMouseUp(int _id, float _x, float _y)
{
	MGUI::InputManager::Instance()->_injectMouseUp(_id, _x, _y);
}

void App::InjectKeyDown(int _key, uchar_t _text)
{
	MGUI::InputManager::Instance()->_injectKeyDown(_key, _text);
}

void App::InjectKeyUp(int _key)
{
	MGUI::InputManager::Instance()->_injectKeyUp(_key);
}

void App::InjectTouchDown(int _id, float _x, float _y)
{
	MGUI::InputManager::Instance()->_injectTouchDown(_id, _x, _y);
}

void App::InjectTouchUp(int _id, float _x, float _y)
{
	MGUI::InputManager::Instance()->_injectTouchUp(_id, _x, _y);
}

void App::InjectTouchMove(int _id, float _x, float _y)
{
	MGUI::InputManager::Instance()->_injectTouchMove(_id, _x, _y);
}

void App::InjectTouchCancel(int _id, float _x, float _y)
{
	MGUI::InputManager::Instance()->_injectTouchCancel(_id, _x, _y);
}
