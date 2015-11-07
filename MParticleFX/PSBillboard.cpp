#include "PSBillboard.h"
#include "PSBillboardSet.h"
#include "MHWBufferManager.h"
#include "MWorld.h"

namespace Rad {

	PS_Billboard::PS_Billboard()
	{
		mRenderOp.vertexDeclarations[0].AddElement(eVertexSemantic::POSITION, eVertexType::FLOAT3);
		mRenderOp.vertexDeclarations[0].AddElement(eVertexSemantic::COLOR, eVertexType::FLOAT4);
		mRenderOp.vertexDeclarations[0].AddElement(eVertexSemantic::TEXCOORD0, eVertexType::FLOAT2);

		mMaterial.cullMode = eCullMode::NONE;
		mMaterial.depthMode = eDepthMode::N_LESS_EQUAL;
		mMaterial.blendMode = eBlendMode::ADD;
	}

	PS_Billboard::~PS_Billboard()
	{
	}

	void PS_Billboard::_getWorldPosition(Float3 & pos)
	{
		if (mParent->IsLocalSpace())
			pos = mParent->GetParent()->GetWorldPosition();
		else
			pos = Float3::Zero;
	}

	void PS_Billboard::_getWorldBound(Aabb & bound)
	{
		bound = mParent->GetBound();
	}

	void PS_Billboard::_getWorldTM(Mat4 & form)
	{
		form = Mat4::Identity;
	}

	void PS_Billboard::_allocBuffer(int quota)
	{
		d_assert (quota > 0);

		VertexBufferPtr vb = HWBufferManager::Instance()->NewVertexBuffer(sizeof(PS_Vertex), quota * 4, eUsage::DYNAMIC_MANAGED);
		IndexBufferPtr ib = HWBufferManager::Instance()->NewIndexBuffer(quota * 6);

		unsigned short * idx = (unsigned short *)ib->Lock(eLockFlag::WRITE);
		for (int i = 0; i < quota; ++i)
		{
			int index = i * 4;

			*idx++ = index + 0; 
			*idx++ = index + 1;
			*idx++ = index + 2;

			*idx++ = index + 2;
			*idx++ = index + 1;
			*idx++ = index + 3;
		}
		ib->Unlock();

		mRenderOp.vertexBuffers[0] = vb;
		mRenderOp.indexBuffer = ib;
		mRenderOp.primCount = 0;
		mRenderOp.primType = ePrimType::TRIANGLE_LIST;
	}

	void PS_Billboard::_updateBuffer()
	{
		mRenderOp.primCount = mParent->GetParticleCount() * 2;
		if (mRenderOp.primCount == 0)
			return ;

		Camera * pCamera = World::Instance()->GetCurrentRenderContext()->GetCamera();

		pCamera->GetWorldRotation().ToAxis(mCamXAxis, mCamYAxis, mCamZAxis);
		mCamPos = pCamera->GetWorldPosition();

		mCommonDir = mParent->GetCommonDirection();
		mCommonUp = mParent->GetCommonUpVector();

		if (mParent->GetBillboardType() == PS_BillboardType::PERPENDICULAR ||
			mParent->GetBillboardType() == PS_BillboardType::PERPENDICULAR_COMMON)
		{
			mCommonDir.TransformQ(mParent->GetParent()->GetWorldRotation());
			mCommonUp.TransformQ(mParent->GetParent()->GetWorldRotation());
		}

		float asecpt = 1;
		if (mParent->IsKeepAspect())
			asecpt = mParent->_getTexture()->_getAscept();

		Mat4 worldTM;
		if (mParent->IsScaleAble())
			worldTM = mParent->GetParent()->GetWorldTM();
		else
			worldTM.MakeTransform(mParent->GetParent()->GetWorldPosition(), mParent->GetParent()->GetWorldRotation(), Float3(1, 1, 1));

		int count = mParent->GetParticleCount();
		d_assert (count * 4 <= mRenderOp.vertexBuffers[0]->GetCount());

		const Float2 & center = mParent->GetBillboardCenter();
		
		PS_Vertex * v = (PS_Vertex *)mRenderOp.vertexBuffers[0]->Lock(eLockFlag::WRITE);
		for (int i = 0; i < count; ++i)
		{
			const Particle * p = mParent->GetParticle(i);

			Float3 xAxis, yAxis;
			float width, height;

			height = Max(0.0f, p->Size.y);
			if (!mParent->IsKeepAspect())
				width = Max(0.0f, p->Size.x);
			else
				width = height * asecpt;

			_getBillboardXYAxis(xAxis, yAxis, p);

			xAxis *= width, yAxis *= height;

			Float3 position = p->Position;
			if (mParent->IsLocalSpace())
			{
				position.TransformA(worldTM);
			}

			v->position = position - xAxis * center.x + yAxis * center.y;
			v->color = p->Color;
			v->uv.x = p->UVRect.x1, v->uv.y = p->UVRect.y1;
			++v;

			v->position = position + xAxis * (1 - center.x) + yAxis * center.y;
			v->color = p->Color;
			v->uv.x = p->UVRect.x2, v->uv.y = p->UVRect.y1;
			++v;

			v->position = position - xAxis * center.x - yAxis * (1 - center.y);
			v->color = p->Color;
			v->uv.x = p->UVRect.x1, v->uv.y = p->UVRect.y2;
			++v;

			v->position = position + xAxis * (1 - center.x) - yAxis * (1 - center.y);
			v->color = p->Color;
			v->uv.x = p->UVRect.x2, v->uv.y = p->UVRect.y2;
			++v;
		}
		mRenderOp.vertexBuffers[0]->Unlock();
	}

	void PS_Billboard::_getBillboardXYAxis(Float3 & xAxis, Float3 & yAxis, const Particle * p)
	{
		int mBillboardType = mParent->GetBillboardType();
		bool mAccurateFacing = mParent->GetAccurateFacing();

		Float3 dir = p->Direction;
		if (mParent->IsLocalSpace())
		{
			dir.TransformQ(mParent->GetParent()->GetWorldRotation());
		}

		switch (mBillboardType)
		{
		case PS_BillboardType::POINT:
			if (mAccurateFacing)
			{
				Float3 vCamDir = p->Position - mCamPos;
				vCamDir.Normalize();

				yAxis = mCamYAxis;
				xAxis = Float3::Cross(yAxis, vCamDir);

				xAxis.Normalize();
			}
			else
			{
				xAxis = mCamXAxis;
				yAxis = mCamYAxis;
			}
			break;

		case PS_BillboardType::ORIENTED:
			if (mAccurateFacing)
			{
				Float3 vCamDir = p->Position - mCamPos;
				vCamDir.Normalize();

				yAxis = dir;
				xAxis = Float3::Cross(yAxis, vCamDir);

				xAxis.Normalize();
			}
			else
			{
				yAxis = dir;
				xAxis = Float3::Cross(yAxis, mCamZAxis);
				
				xAxis.Normalize();
			}
			break;

		case PS_BillboardType::ORIENTED_COMMON:
			if (mAccurateFacing)
			{
				Float3 vCamDir = p->Position - mCamPos;
				vCamDir.Normalize();

				yAxis = mCommonDir;
				xAxis = Float3::Cross(yAxis, vCamDir);

				xAxis.Normalize();
			}
			else
			{
				yAxis = mCommonDir;
				xAxis = Float3::Cross(yAxis, mCamZAxis);

				xAxis.Normalize();
			}
			break;

		case PS_BillboardType::PERPENDICULAR:
			{
				xAxis = Float3::Cross(mCommonUp, dir);
				yAxis = Float3::Cross(dir, xAxis);
				
				xAxis.Normalize();
				yAxis.Normalize();
			}
			break;

		case PS_BillboardType::PERPENDICULAR_COMMON:
			{
				xAxis = Float3::Cross(mCommonUp, mCommonDir);
				yAxis = mCommonDir;

				xAxis.Normalize();
			}
			break;
		}


		if (p->Rotation.x != 0)
		{
			Float3 axis = Float3::Cross(xAxis, yAxis);

			Quat q;
			q.FromAxis(axis, Math::DegreeToRadian(p->Rotation.x));

			xAxis.TransformQ(q);
			yAxis.TransformQ(q);
		}
	}

	void PS_Billboard::_update()
	{
		int blendMode = mParent->GetBlendMode();

		mMaterial.depthMode = eDepthMode::N_LESS_EQUAL;

		if (blendMode == PS_BlendMode::ADD)
		{
			mMaterial.blendMode = eBlendMode::ADD;
		}
		else if (blendMode == PS_BlendMode::ALPHA_BLEND)
		{
			mMaterial.blendMode = eBlendMode::ALPHA_BLEND;
		}
		else if (blendMode == PS_BlendMode::COLOR_BLEND)
		{
			mMaterial.blendMode = eBlendMode::COLOR_BLEND;
		}
		else
		{
			mMaterial.blendMode = eBlendMode::OPACITY;
			mMaterial.depthMode = eDepthMode::LESS_EQUAL;
		}

		if (!mParent->IsDepthCheck())
		{
			mMaterial.depthMode = eDepthMode::NONE;
		}

		mMaterial.maps[eMapType::DIFFUSE] = mParent->_getTexture();

		mShaderFX = mParent->_getShaderFX();
		SetRenderCallBack(eRenderCallBack::SHADER, mParent->GetShader().c_ptr());
	}
	
}