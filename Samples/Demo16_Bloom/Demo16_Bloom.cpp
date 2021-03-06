#include "App.h"
#include "MBloom.h"

Mesh * gMesh = NULL;
Bloom * gBloom = NULL;

void LookMesh(Mesh * entity)
{
	Aabb bound = entity->GetWorldAabb();

	float size = 0;
	size = Max(size, bound.GetSize().x);
	size = Max(size, bound.GetSize().y);
	size = Max(size, bound.GetSize().z);

	Node * pCamera = World::Instance()->MainCamera();
	pCamera->SetPosition(bound.GetCenter() + Float3(0, 1, -2) * size);
	pCamera->SetDirection(bound.GetCenter() - pCamera->GetPosition());
}

class Demo16_Bloom : public App
{
public:
	Demo16_Bloom() {}
	virtual ~Demo16_Bloom() {}

	virtual void OnSetupResource()
	{
#ifdef M_PLATFORM_WIN32
		ResourceManager::Instance()->AddArchive(new FilePathArchive("../Core"));
		ResourceManager::Instance()->AddArchive(new FilePathArchive("../Sample"));
		ResourceManager::Instance()->AddArchive(new FilePathArchive("../Sample/RenderProcess"));
#endif

#ifdef M_PLATFORM_ANDROID
		ResourceManager::Instance()->AddArchive(new APKArchive("Core"));
		ResourceManager::Instance()->AddArchive(new APKArchive("Sample"));
#endif
	}

	virtual void OnInit()
	{
		MGUI::FontManager::Instance()->Load("Sample.font");

		// Setup Main Light
		World::Instance()->MainLight()->SetDirection(Float3(-1, 0, 1));

		// Create Mesh
		MeshSourcePtr pMeshSource = MeshManager::Instance()->LoadMesh("Mesh/fox.mesh");

		gMesh = new Mesh;
		gMesh->SetSource(pMeshSource);
		gMesh->LoadAnimation("Idle1", "Mesh/fox_idle1.anim");
		gMesh->SetLighting(true);

		gMesh->PlayAnimation("Idle1");

		LookMesh(gMesh);

		Viewport vp = World::Instance()->MainRenderContext()->GetViewport();

		RenderTargetPtr pRenderTarget = HWBufferManager::Instance()->NewRenderTarget(vp.w, vp.h, ePixelFormat::R8G8B8);
		DepthBufferPtr pDepthBuffer = HWBufferManager::Instance()->NewDepthBuffer(vp.w, vp.h, ePixelFormat::D24);

		World::Instance()->MainRenderContext()->SetRenderTarget(0, pRenderTarget);
		World::Instance()->MainRenderContext()->SetDepthBuffer(pDepthBuffer);

		gBloom = new Bloom(World::Instance()->MainRenderContext());

		World::Instance()->E_RenderEnd += new cListener0<Demo16_Bloom>(this, &Demo16_Bloom::OnPostRender);
	}

	virtual void OnUpdate()
	{
	}

	virtual void OnShutdown()
	{
		delete gMesh;
		delete gBloom;
	}

	virtual void OnPause()
	{
	}

	virtual void OnResume()
	{
	}

	void OnPostRender()
	{
		RenderTargetPtr pRenderTarget = World::Instance()->MainRenderContext()->GetRenderTarget(0);
		if (pRenderTarget != NULL)
		{
			gBloom->OnRender();

			RenderHelper::Instance()->DrawSumit(
				World::Instance()->MainRenderContext()->GetViewport(),
				pRenderTarget->GetTexture().c_ptr());
		}
	}
};

void CreateApplication(App ** ppApp)
{
	*ppApp = new Demo16_Bloom;
}
