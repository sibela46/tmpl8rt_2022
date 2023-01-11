#pragma once

#include <direct.h>

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

		Ox4 = _mm_set_ps1(origin.x);
		Oy4 = _mm_set_ps1(origin.y);
		Oz4 = _mm_set_ps1(origin.z);
		Dx4 = _mm_set_ps1(direction.x);
		Dy4 = _mm_set_ps1(direction.y);
		Dz4 = _mm_set_ps1(direction.z);
		rDx4 = _mm_set_ps1(1 / direction.x);
		rDy4 = _mm_set_ps1(1 / direction.y);
		rDz4 = _mm_set_ps1(1 / direction.z);
		t4 = _mm_set_ps1(distance);
	}
	float3 IntersectionPoint() const { return (O + t * D); }
	__m128 IntersectionPointX() { return _mm_add_ps(Ox4, _mm_mul_ps(t4, Dx4)); }
	__m128 IntersectionPointY() { return _mm_add_ps(Oy4, _mm_mul_ps(t4, Dy4)); }
	__m128 IntersectionPointZ() { return _mm_add_ps(Oz4, _mm_mul_ps(t4, Dz4)); }

	// ray data
	union { struct { float3 O; float d0; }; __m128 O4; };
	union { struct { float3 D; float d1; }; __m128 D4; };
	union { struct { float3 rD; float d2; }; __m128 rD4; };

	float t = 1e34f;
	int objIdx = -1;
	bool inside = false;
	Material objMaterial;
	ObjectType objType;
	float3 normal;
	__m128 Ox4, Oy4, Oz4;
	__m128 Dx4, Dy4, Dz4;
	__m128 rDx4, rDy4, rDz4;
	__m128 t4;
};

}