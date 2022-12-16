#include "precomp.h"

Plane::Plane(int id, const float3& n, float d, Material m, TextureMap* t)
{
	distance = d;
	normal = n;
	index = id;
	material = m;
	texture = t;
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
		ray.objType = ObjectType::PLANE;
		ray.normal = normal;
	}
}

float3 Plane::GetUVCoords()
{
	float3 a = cross(normal, float3(1, 0, 0));
	float3 b = cross(normal, float3(0, 1, 0));

	float3 max_ab = dot(a, a) < dot(b, b) ? b : a;

	float3 c = cross(normal, float3(0, 0, 1));

	return normalize(dot(max_ab, max_ab) < dot(c, c) ? c : max_ab);
}

float3 Plane::GetTexture(const float3& I, const float3& N)
{
	if (texture == nullptr) return material.colour;
	float3 u = GetUVCoords();
	float3 v = cross(N, GetUVCoords());
	return texture->GetColourAt(fmod(abs(dot(u, I)), 1.f), fmod(abs(dot(v, I)), 1.f));
}