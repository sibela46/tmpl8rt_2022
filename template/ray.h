#pragma once

#include <direct.h>

#define SPEEDTRIX

namespace Tmpl8 {

__declspec(align(64)) class Ray
{
public:
	Ray() = default;
	Ray(float3 origin, float3 direction, float distance = 1e34f)
	{
		O = origin, D = direction, t = distance;
		// calculate reciprocal ray direction for triangles and AABBs
		rD = float3(1 / D.x, 1 / D.y, 1 / D.z);
#ifdef SPEEDTRIX
		d0 = d1 = d2 = 0;
#endif
	}
	float3 IntersectionPoint() { return (O + t * D); }
	// ray data
#ifndef SPEEDTRIX
	float3 O, D, rD;
#else
	union { struct { float3 O; float d0; }; __m128 O4; };
	union { struct { float3 D; float d1; }; __m128 D4; };
	union { struct { float3 rD; float d2; }; __m128 rD4; };
#endif
	float t = 1e34f;
	int objIdx = -1;
	bool inside = false;
	Material objMaterial;
	ObjectType objType;
	float3 normal;
};

}