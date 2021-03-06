#include "MMeshManager.h"
#include "MHWBufferManager.h"
#include "MResourceManager.h"
#include "MRenderHelper.h"
#include "MMeshShader.h"
#include "MWorld.h"
#include "MRoot.h"

namespace Rad {

	ImplementSingleton(MeshManager);

	MeshManager::MeshManager()
	{
	}

	MeshManager::~MeshManager()
	{
		d_assert (mMeshSourceMap.Empty() && "All mesh has already delete?");
	}

	MeshSourcePtr MeshManager::_find(Hash2 hash, const String & name)
	{
		int i = mMeshSourceMap.Find(hash);
		
		d_assert (i == - 1 || mMeshSourceMap[i].value->GetName() == name);

		return i != -1 ? mMeshSourceMap[i].value : NULL;
	}

	MeshSourcePtr MeshManager::GetMesh(const String & meshName)
	{
		return _find(Hash2(meshName.c_str()), meshName);
	}

	MeshSourcePtr MeshManager::NewMesh(const String & meshName)
	{
		d_assert (GetMesh(meshName) == NULL);

		MeshSource * pMesh = new MeshSource(meshName, "");

		mMeshSourceMap.Insert(Hash2(meshName.c_str()), pMesh);

		return pMesh;
	}

	MeshSourcePtr MeshManager::LoadMesh(const String & filename, float priority)
	{
		Hash2 hash(filename.c_str());

		MeshSourcePtr p = _find(hash, filename);
		if (p == NULL)
		{
			p = new MeshSource(filename, filename);
			p->SetPriority(priority);

			mMeshSourceMap.Insert(hash, p.c_ptr());
		}

		if (priority < 0)
		{
			p->EnsureLoad();
		}
		else
		{
			p->Load();
		}

		return p;
	}

	void MeshManager::_delMesh(MeshSource * pMesh)
	{
		Hash2 hash(pMesh->GetName().c_str());
		int i = mMeshSourceMap.Find(hash);

		d_assert (i != -1 && pMesh->GetName() == mMeshSourceMap[i].value->GetName());

		delete mMeshSourceMap[i].value;
		mMeshSourceMap.Erase(i);
	}

	Mesh * MeshManager::NewBox(float width, float height, float depth, const Float3 & offset)
	{
		Mesh * pMesh = new Mesh;
		SubMesh * sm = pMesh->NewSubMesh();
    
		int iVertexCount = 4 * 6;
		int iIndexCount = 12 * 3;
		int iPrimCount = 12;

		sm->GetRenderOp()->vertexDeclarations[0].AddElement(eVertexSemantic::POSITION, eVertexType::FLOAT3);
		sm->GetRenderOp()->vertexDeclarations[0].AddElement(eVertexSemantic::NORMAL, eVertexType::FLOAT3);
		sm->GetRenderOp()->vertexDeclarations[0].AddElement(eVertexSemantic::TEXCOORD0, eVertexType::FLOAT2);

		VertexBufferPtr buffer = HWBufferManager::Instance()->NewVertexBuffer(sizeof(float) * 8, iVertexCount);

		const float half_w = width * 0.5f;
		const float half_h = height * 0.5f;
		const float half_d = depth * 0.5f;
		float * v = (float *)buffer->Lock(eLockFlag::WRITE);
		{
#define d_pos(i) *v++ = all_pos[i].x, *v++ = all_pos[i].y, *v++ = all_pos[i].z
#define d_normal(i) *v++ = all_normal[i].x, *v++ = all_normal[i].y, *v++ = all_normal[i].z
#define d_uv(i) *v++ = all_uv[i].x, *v++ = all_uv[i].y

			Float3 all_pos[8] = {
				Float3(-half_w,  half_h, -half_d) + offset,
				Float3( half_w,  half_h, -half_d) + offset,
				Float3(-half_w, -half_h, -half_d) + offset,
				Float3( half_w, -half_h, -half_d) + offset,

				Float3(-half_w,  half_h,  half_d) + offset,
				Float3( half_w,  half_h,  half_d) + offset,
				Float3(-half_w, -half_h,  half_d) + offset,
				Float3( half_w, -half_h,  half_d) + offset,
			};

			Float3 all_normal[6] = {
				Float3( 0,  0, -1),
				Float3( 0,  0,  1),

				Float3(-1,  0,  0),
				Float3( 1,  0,  0),

				Float3( 0,  1,  0),
				Float3( 0,  -1,  0)
			};

			Float2 all_uv[4] = {
				Float2(0, 0),
				Float2(1, 0),
				Float2(0, 1),
				Float2(1, 1)
			};

			// front
			d_pos(0), d_normal(0), d_uv(0);
			d_pos(1), d_normal(0), d_uv(1);
			d_pos(2), d_normal(0), d_uv(2);
			d_pos(3), d_normal(0), d_uv(3);

			// back
			d_pos(5), d_normal(1), d_uv(0);
			d_pos(4), d_normal(1), d_uv(1);
			d_pos(7), d_normal(1), d_uv(2);
			d_pos(6), d_normal(1), d_uv(3);

			// left
			d_pos(4), d_normal(2), d_uv(0);
			d_pos(0), d_normal(2), d_uv(1);
			d_pos(6), d_normal(2), d_uv(2);
			d_pos(2), d_normal(2), d_uv(3);

			// right
			d_pos(1), d_normal(3), d_uv(0);
			d_pos(5), d_normal(3), d_uv(1);
			d_pos(3), d_normal(3), d_uv(2);
			d_pos(7), d_normal(3), d_uv(3);

			// top
			d_pos(4), d_normal(4), d_uv(0);
			d_pos(5), d_normal(4), d_uv(1);
			d_pos(0), d_normal(4), d_uv(2);
			d_pos(1), d_normal(4), d_uv(3);

			// bottom
			d_pos(2), d_normal(5), d_uv(0);
			d_pos(3), d_normal(5), d_uv(1);
			d_pos(6), d_normal(5), d_uv(2);
			d_pos(7), d_normal(5), d_uv(3);

#undef d_pos
#undef d_normal
#undef d_uv

		}
		buffer->Unlock();

		sm->GetRenderOp()->vertexBuffers[0] = buffer;

		IndexBufferPtr ibuffer = HWBufferManager::Instance()->NewIndexBuffer(iIndexCount);
		short * indices = (short *)ibuffer->Lock(eLockFlag::WRITE);
		for (int i = 0; i < 6; ++i)
		{
			int idx = i * 4;

			*indices++ = idx + 0;
			*indices++ = idx + 1;
			*indices++ = idx + 2;

			*indices++ = idx + 2;
			*indices++ = idx + 1;
			*indices++ = idx + 3;
		}
		ibuffer->Unlock();

		sm->GetRenderOp()->indexBuffer = ibuffer;

		sm->GetRenderOp()->primCount = iPrimCount;
		sm->GetRenderOp()->primType = ePrimType::TRIANGLE_LIST;

		sm->GetMaterial()->maps[eMapType::DIFFUSE] = RenderHelper::Instance()->GetWhiteTexture();

		pMesh->SetLocalAabb(Aabb(Float3(-half_w, -half_h, -half_d) + offset, Float3(half_w, half_h, half_d) + offset));

		return pMesh;
	}

	Mesh * MeshManager::NewSphere(short rings, short segments, float radius, const Float3 & offset)
	{
		Mesh * pMesh = new Mesh;
		SubMesh * sm = pMesh->NewSubMesh();

		int iVertexCount = (rings + 1) * (segments + 1);
		int iIndexCount = rings * segments * 6;
		int iPrimCount = iIndexCount / 3;

		d_assert(iIndexCount < 65536);

		sm->GetRenderOp()->vertexDeclarations[0].AddElement(eVertexSemantic::POSITION, eVertexType::FLOAT3);
		sm->GetRenderOp()->vertexDeclarations[0].AddElement(eVertexSemantic::NORMAL, eVertexType::FLOAT3);

		VertexBufferPtr buffer = HWBufferManager::Instance()->NewVertexBuffer(24, iVertexCount);

		float * vert = (float *)buffer->Lock(eLockFlag::WRITE);
		{
			float fTileRingAngle = (PI / rings);
			float fTileSegAngle = (PI * 2 / segments);
			float r;
			short i, j;
			Float3 pos, normal;

			for (i = 0; i <= rings; ++i)
			{
				r = radius * Math::Sin(i * fTileRingAngle);
				pos.y = radius * Math::Cos(i * fTileRingAngle);

				for (j = 0; j <= segments; ++j)
				{
					pos.x = r * Math::Cos(j * fTileSegAngle);
					pos.z = r * Math::Sin(j * fTileSegAngle);
					normal = pos; normal.Normalize();

					*vert++ = pos.x + offset.x;
					*vert++ = pos.y + offset.y;
					*vert++ = pos.z + offset.z;
					*vert++ = normal.x;
					*vert++ = normal.y;
					*vert++ = normal.z;
				}

			}
		}
		buffer->Unlock();

		IndexBufferPtr ibuffer = HWBufferManager::Instance()->NewIndexBuffer(iIndexCount);
		short * indices = (short *)ibuffer->Lock(eLockFlag::WRITE);
		{
			short row = 0, row_n = 0;
			short i, j;

			for (i = 0; i < rings; ++i)
			{
				row_n = row + segments + 1;

				for (j = 0; j < segments; ++j)
				{
					*indices++ = row + j;
					*indices++ = row + j + 1;
					*indices++ = row_n + j;

					*indices++ = row_n + j;
					*indices++ = row + j + 1;
					*indices++ = row_n + j + 1;

				}

				row += segments + 1;
			}
		}
		ibuffer->Unlock();
		
		sm->GetRenderOp()->vertexBuffers[0] = buffer;
		sm->GetRenderOp()->indexBuffer = ibuffer;
		sm->GetRenderOp()->primCount = iPrimCount;
		sm->GetRenderOp()->primType= ePrimType::TRIANGLE_LIST;

		sm->GetMaterial()->maps[eMapType::DIFFUSE] = RenderHelper::Instance()->GetWhiteTexture();

		pMesh->SetLocalAabb(Aabb(Float3(-radius, -radius, -radius) + offset, Float3(radius, radius, radius) + offset));

		return pMesh;
	}

	Mesh * MeshManager::NewCylinder(float radius, float height, int rings, const Float3 & offset)
	{
		Mesh * pMesh = new Mesh;
		SubMesh * sm = pMesh->NewSubMesh();

		int iVertexCount = (rings + 1) * 2;
		int iIndexCount = rings * 6;
		int iPrimCount = iIndexCount / 3;
		float half_h = height / 2;

		d_assert(iIndexCount < 65536);

		sm->GetRenderOp()->vertexDeclarations[0].AddElement(eVertexSemantic::POSITION, eVertexType::FLOAT3);
		sm->GetRenderOp()->vertexDeclarations[0].AddElement(eVertexSemantic::NORMAL, eVertexType::FLOAT3);

		VertexBufferPtr buffer = HWBufferManager::Instance()->NewVertexBuffer(24, iVertexCount);

		float * vert = (float *)buffer->Lock(eLockFlag::WRITE);
		{
			float fTileRingAngle = (2 * PI / rings);
			float x, z, rads;

			for (int i = 0; i <= rings; ++i)
			{
				rads = i * fTileRingAngle;

				Math::SinCos(rads, z, x);

				x *= radius;
				z *= radius;

				*vert++ = x + offset.x;
				*vert++ = -half_h + offset.y;
				*vert++ = z + offset.z;
				*vert++ = x;
				*vert++ = 0;
				*vert++ = z;

				*vert++ = x + offset.x;
				*vert++ = half_h + offset.y;
				*vert++ = z + offset.z;
				*vert++ = x;
				*vert++ = 0;
				*vert++ = z;
			}
		}
		buffer->Unlock();

		IndexBufferPtr ibuffer = HWBufferManager::Instance()->NewIndexBuffer(iIndexCount);
		short * indices = (short *)ibuffer->Lock(eLockFlag::WRITE);
		{
			for (short i = 0; i < rings; ++i)
			{
				int j = i * 2;

				*indices++ = j;
				*indices++ = j + 1;
				*indices++ = j + 2;

				*indices++ = j + 2;
				*indices++ = j + 1;
				*indices++ = j + 3;
			}
		}
		ibuffer->Unlock();

		sm->GetRenderOp()->vertexBuffers[0] = buffer;
		sm->GetRenderOp()->indexBuffer = ibuffer;
		sm->GetRenderOp()->primType = ePrimType::TRIANGLE_LIST;
		sm->GetRenderOp()->primCount = iPrimCount;

		sm->GetMaterial()->cullMode = eCullMode::NONE;
		sm->GetMaterial()->maps[eMapType::DIFFUSE] = RenderHelper::Instance()->GetWhiteTexture();

		pMesh->SetLocalAabb(Aabb(Float3(-radius, -half_h, -radius) + offset, Float3(radius, half_h, radius) + offset));

		return pMesh;
	}

	Mesh * MeshManager::NewCone(float radius, float height, int rings, const Float3 & offset)
	{
		Mesh * pMesh = new Mesh;
		SubMesh * sm = pMesh->NewSubMesh();
		float half_h = height / 2;

		int iVertexCount = 1 + (rings + 1);
		int iIndexCount = rings * 3;
		int iPrimCount = iIndexCount / 3;

		d_assert(iIndexCount < 65536);

		sm->GetRenderOp()->vertexDeclarations[0].AddElement(eVertexSemantic::POSITION, eVertexType::FLOAT3);
		sm->GetRenderOp()->vertexDeclarations[0].AddElement(eVertexSemantic::NORMAL, eVertexType::FLOAT3);

		VertexBufferPtr buffer = HWBufferManager::Instance()->NewVertexBuffer(24, iVertexCount);

		float * vert = (float *)buffer->Lock(eLockFlag::WRITE);
		{
			float fTileRingAngle = (2 * PI / rings);
			float x, z, rads;

			*vert++ = 0 + offset.x;
			*vert++ = half_h + offset.x;
			*vert++ = 0 + offset.z;
			*vert++ = 0;
			*vert++ = 1;
			*vert++ = 0;

			for (int i = 0; i <= rings; ++i)
			{
				rads = i * fTileRingAngle;

				Math::SinCos(rads, z, x);

				x *= radius;
				z *= radius;

				*vert++ = x + offset.x;
				*vert++ = -half_h + offset.y;
				*vert++ = z + offset.z;
				*vert++ = x;
				*vert++ = 0;
				*vert++ = z;
			}
		}
		buffer->Unlock();

		IndexBufferPtr ibuffer = HWBufferManager::Instance()->NewIndexBuffer(iIndexCount);
		short * indices = (short *)ibuffer->Lock(eLockFlag::WRITE);
		{
			short n = rings + 1;
			for (short i = 0; i < rings; ++i)
			{
				*indices++ = i + 1;
				*indices++ = 0;
				*indices++ = i + 2;
			}
		}
		ibuffer->Unlock();

		sm->GetRenderOp()->vertexBuffers[0] = buffer;
		sm->GetRenderOp()->indexBuffer = ibuffer;
		sm->GetRenderOp()->primType = ePrimType::TRIANGLE_LIST;
		sm->GetRenderOp()->primCount = iPrimCount;

		sm->GetMaterial()->cullMode = eCullMode::NONE;
		sm->GetMaterial()->maps[eMapType::DIFFUSE] = RenderHelper::Instance()->GetWhiteTexture();

		pMesh->SetLocalAabb(Aabb(Float3(-radius, -half_h, -radius) + offset, Float3(radius, half_h, radius) + offset));

		return pMesh;
	}

	Mesh * MeshManager::NewPlane(const Plane & plane, float width, float height, int iTileW, int iTileH)
	{
		Float3 a(-width * 0.5f, 0.0f, height * 0.5f);
		Float3 b(width * 0.5f, 0.0f, height * 0.5f);
		Float3 c(-width * 0.5f, 0.0f, -height * 0.5f);
		Float3 d(width * 0.5f, 0.0f, -height * 0.5f);

		Float3 normal = plane.normal;
		Float3 trans = normal * -plane.d;
		Float3 axis;

		float dot = normal.Dot(Float3(0, 1, 0));

		if (Math::Abs(dot) < (1 - DEFAULT_EPSILON))
		{
			axis = Float3::Cross(Float3(0, 1, 0), normal);

			float rads = Math::FastACos(normal.Dot(Float3(0, 1, 0)));

			Quat quat;
			quat.FromAxis(axis, rads);

			a.TransformQ(quat);
			b.TransformQ(quat);
			c.TransformQ(quat);
			d.TransformQ(quat);
		}

		a += trans;
		b += trans;
		c += trans;
		d += trans;

		int iVertexCount = (iTileW + 1) * (iTileH + 1);
		int iIndexCount = iTileW * iTileH * 6;
		int iPrimCount = iIndexCount / 3;

		d_assert(iIndexCount < 10000);

		Mesh * pMesh = new Mesh;
		SubMesh * sm = pMesh->NewSubMesh();

		sm->GetRenderOp()->vertexDeclarations[0].AddElement(eVertexSemantic::POSITION, eVertexType::FLOAT3);
		sm->GetRenderOp()->vertexDeclarations[0].AddElement(eVertexSemantic::NORMAL, eVertexType::FLOAT3);
		sm->GetRenderOp()->vertexDeclarations[0].AddElement(eVertexSemantic::TEXCOORD0, eVertexType::FLOAT2);

		VertexBufferPtr buffer = HWBufferManager::Instance()->NewVertexBuffer(sizeof(float) * 8, iVertexCount);

		float * vert;
		vert = (float *)buffer->Lock(eLockFlag::WRITE);
		{
			Float3 dw = (b - a) / (float)iTileW;
			Float3 dh = (c - a) / (float)iTileH;
			Float3 pos_l = a;
			Float3 pos;

			float start_u = 0;
			float start_v = 0;
			float end_u = 1.0f;
			float end_v = 1.0f;
			float inc_u = 1.0f / (float)iTileW;
			float inc_v = 1.0f / (float)iTileH;
			float u = start_u;
			float v = start_v;

			for (int i = 0; i < iTileH + 1; ++i)
			{
				pos = pos_l;
				u = start_u;

				for (int j = 0; j < iTileW + 1; ++j)
				{
					*vert++ = pos.x;
					*vert++ = pos.y;
					*vert++ = pos.z;

					*vert++ = normal.x;
					*vert++ = normal.y;
					*vert++ = normal.z;

					*vert++ = u;
					*vert++ = v;

					pos += dw;
					u += inc_u;
				}

				pos_l += dh;
				v += inc_v;
			}
		}
		buffer->Unlock();

		sm->GetRenderOp()->vertexBuffers[0] = buffer;

		IndexBufferPtr ibuffer = HWBufferManager::Instance()->NewIndexBuffer(iIndexCount);

		short * indices;
		indices = (short *)ibuffer->Lock(eLockFlag::WRITE);
		{
			int i, j;
			int cur_r = 0;
			int next_r = 0;

			for (i = 0; i < iTileH; ++i)
			{
				next_r = cur_r + iTileW + 1;
				for (j = 0; j < iTileW; ++j)
				{
					*indices++ = cur_r + j;
					*indices++ = cur_r + j + 1;
					*indices++ = next_r + j;

					*indices++ = next_r + j;
					*indices++ = cur_r + j + 1;
					*indices++ = next_r + j + 1;
				}

				cur_r += iTileW + 1;
			}
		}
		ibuffer->Unlock();

		sm->GetRenderOp()->indexBuffer = ibuffer;
		sm->GetRenderOp()->primType = ePrimType::TRIANGLE_LIST;
		sm->GetRenderOp()->primCount = iPrimCount;

		sm->GetMaterial()->maps[eMapType::DIFFUSE] = RenderHelper::Instance()->GetWhiteTexture();

		Float3 vMin, vMax;
		
		vMin = a;
		vMin = Float3::Minimum(vMin, b);
		vMin = Float3::Minimum(vMin, c);
		vMin = Float3::Minimum(vMin, d);

		vMax = a;
		vMax = Float3::Maximum(vMax, b);
		vMax = Float3::Maximum(vMax, c);
		vMax = Float3::Maximum(vMax, d);

		pMesh->SetLocalAabb(Aabb(vMin, vMax));

		return pMesh;
	}

}