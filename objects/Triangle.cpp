#include "precomp.h"

Triangle::Triangle(int id, const float4& vertex1, const float4& vertex2, const float4& vertex3, Material m, TextureMap* t)
{
	v0 = vertex1, v1 = vertex2, v2 = vertex3;
	material = m;
	index = id;
	texture = t;
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
	u = f * dot(s, h);
	if (u < 0.0 || u > 1.0) return;
	float3 q = cross(s, edge1);
	v = f * dot(ray.D, q);
	if (v < 0.0 || u + v > 1.0) return;
	float t = f * dot(edge2, q);
	if (t > EPSILON)
	{
		ray.t = t;
		ray.objIdx = index;
		ray.objMaterial = material;
		ray.objType = ObjectType::TRIANGLE;
		return;
	}
	return;
}

float3 Triangle::GetNormal(const float3& I)
{
	return normalize(cross(v1 - v0, v2 - v0));
}

float3 Triangle::GetTexture(const float3& I, const float3& N)
{
	if (texture == nullptr) return material.colour;
	return texture->GetColourAt(u, v);
}
