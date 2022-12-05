#include "precomp.h"

Plane::Plane(int id, const float3& n, float d, Material m)
{
	distance = d;
	normal = n;
	index = id;
	material = m;
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
	}
}

float3 Plane::GetDirectLight(Light* light, const float3& I, const float3& N)
{
	float3 dirToLight = (light->GetPosition() - I);
	float dotProduct = max(0.f, dot(normalize(dirToLight), N));
	return light->GetEmission() * light->GetColour() * dotProduct * (1 / PI);
}

float3 Plane::GetSpecularColour(Light* light, const float3& I, const float3& N, const float3& D)
{
	float3 distToLight = normalize(light->position - I);
	float A = 4 * PI * dot(distToLight, distToLight);
	float3 B = light->GetColour() / A;
	float3 reflected = normalize(reflect(-distToLight, N));
	return pow(-dot(reflected, D), 20.0f);
}

float3 Plane::GetAlbedo(Light* light, const float3& I, const float3& N, const float3& D)
{
	return material.colour * material.Kd + material.Ks * GetSpecularColour(light, I, N, D);
}