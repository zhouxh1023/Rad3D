/*
*	PSBillboardSet
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "PSTypes.h"

namespace Rad {

	class PS_BillboardSet;

	class FX_ENTRY PS_Billboard : public RenderObject
	{
		friend class PS_BillboardSet;

	public:
		PS_Billboard();
		virtual ~PS_Billboard();

		virtual void 
			_getWorldPosition(Float3 & pos);
		virtual void 
			_getWorldBound(Aabb & bound);
		virtual void 
			_getWorldTM(Mat4 & form);

		void 
			_allocBuffer(int quota);
		void 
			_updateBuffer();
		void 
			_getBillboardXYAxis(Float3 & xAxis, Float3 & yAxis, const Particle * p);

		void
			_update();

	protected:
		PS_BillboardSet * mParent;
		Float3 mCamPos, mCamXAxis, mCamYAxis, mCamZAxis;
		Float3 mCommonDir, mCommonUp;
	};

}