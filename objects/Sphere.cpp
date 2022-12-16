#include "precomp.h"

Sphere::Sphere(int id, const float3& p, float r, Material m, TextureMap* t)
{
	index = id;
	radius = r;
	position = p;
	material = m;
	texture = t;
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
		float3 I = ray.O + t * ray.D;
		float3 normal = GetNormal(I);
		double u = 0.5 - atan2(normal.z, normal.x) * (1/PI);
		double v = 0.5 - asin(normal.y) * (1/PI);
		uvCoords = float2(u, v);
		ray.t = t, ray.objIdx = index, ray.objMaterial = material, ray.objType = ObjectType::SPHERE, ray.normal = normal, ray.inside = dot(ray.D, normal) > 0;
		return;
	}
	t = d - b;
	if (t < ray.t && t > 0)
	{
		float3 I = ray.O + t * ray.D;
		float3 normal = GetNormal(I);
		double u = 0.5 - atan2(normal.z, normal.x) * (1 / PI);
		double v = 0.5 - asin(normal.y) * (1 / PI);
		uvCoords = float2(u, v);
		ray.t = t, ray.objIdx = index, ray.objMaterial = material, ray.objType = ObjectType::SPHERE, ray.normal = normal, ray.inside = dot(ray.D, normal) > 0;
		return;
	}
}

float3 Sphere::GetTexture(const float3& I, const float3& N)
{
	if (texture == nullptr) return material.colour;

	auto phi = atan2(-N.z, N.x) + PI;
	auto theta = acos(-N.y);

	float u = phi / (2 * PI);
	float v = theta / PI;
	return texture->GetColourAt(u, v);
}
