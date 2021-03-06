/*
*	PSParticle
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MMath2.h"
#include "MField.h"

#define PS_MAX_RENDER 4
#define PS_MAX_EMITTER 6
#define PS_MAX_MODIFIER 6

namespace Rad {

#define PS_PARTICLE_PARAM_SIZE 4

	struct Particle
	{
		DECLARE_ALLOC();

		Float3 Position;
		Float3 Direction;
		Float3 Rotation;
		Float3 Size;
		Float4 Color;
		RectF  UVRect;
		float  Speed;
		float  Life;

		Float3 InitSize;
		Float4 InitColor;
		Float2 InitLife;

		Field<IntFloatUnion, PS_PARTICLE_PARAM_SIZE> Param;

		void * InternalData;
	};
}
