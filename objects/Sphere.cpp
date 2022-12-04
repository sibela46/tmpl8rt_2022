#include "precomp.h"

Sphere::Sphere(int id, const float3& p, float r, Material m) : Object(id, p, m)
{
	radius = r;
}

float3 Sphere::GetNormal(const float3& I)
{
	return normalize(I - position);
}

void Sphere::Intersect(Ray& ray)
{
	float3 oc = ray.O - position;
	float b = dot(oc, ray.D);
	float c = dot(oc, oc) - radius * radius;
	float t, d = b * b - c;
	if (d <= 0) return;
	d = sqrtf(d), t = -b - d;
	if (t < ray.t && t > 0)
	{
		ray.t = t, ray.objIdx = index, ray.objMaterial = material;
		return;
	}
	t = d - b;
	if (t < ray.t && t > 0)
	{
		ray.t = t, ray.objIdx = index, ray.objMaterial = material;
		return;
	}
}