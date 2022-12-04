#include "precomp.h"

Plane::Plane(int id, const float3& n, float d, Material m) : Object(id, n, m)
{
	distance = d;
	normal = n;
}

float3 Plane::GetNormal(const float3& I)
{
	return normal;
}

void Plane::Intersect(Ray& ray)
{
	float t = -(dot(ray.O, normal) + distance) / (dot(ray.D, normal));
	if (t < ray.t && t > 0)
	{
		ray.t = t;
		ray.objIdx = index;
		ray.objMaterial = material;
	}
}