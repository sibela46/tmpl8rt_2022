#include "precomp.h"

Triangle::Triangle(int id, const float3& vertex1, const float3& vertex2, const float3& vertex3, Material m) : Object(id, vertex1, m)
{
	v0 = vertex1, v1 = vertex2, v2 = vertex3;
}

void Triangle::Intersect(Ray& ray)
{
	float3 edge1 = v1 - v0;
	float3 edge2 = v2 - v0;
	float3 h = cross(ray.D, edge2);
	float a = dot(edge1, h);
	if (a > -EPSILON && a < EPSILON) return; // the ray is parallel to the triangle	
	float f = 1.0 / a;
	float3 s = ray.O - v0;
	float u = f * dot(s, h);
	if (u < 0.0 || u > 1.0) return;
	float3 q = cross(s, edge1);
	float v = f * dot(ray.D, q);
	if (v < 0.0 || u + v > 1.0) return;
	float t = f * dot(edge2, q);
	if (t > EPSILON)
	{
		ray.t = t;
		ray.objIdx = index;
		ray.objMaterial = material;
		return;
	}
	return;
}

float3 Triangle::GetNormal(const float3& I)
{
	float3 normal = normalize(cross(v1 - v0, v2 - v0));
	return normal;
}

