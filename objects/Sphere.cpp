#include "precomp.h"

Sphere::Sphere(int id, const float3& p, float r, Material m)
{
	index = id;
	radius = r;
	position = p;
	material = m;
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
		ray.t = t, ray.objIdx = index, ray.objMaterial = material, ray.objType = ObjectType::SPHERE;
		return;
	}
	t = d - b;
	if (t < ray.t && t > 0)
	{
		ray.t = t, ray.objIdx = index, ray.objMaterial = material, ray.objType = ObjectType::SPHERE;
		return;
	}
}

float3 Sphere::GetDirectLight(Light* light, const float3& I, const float3& N)
{
	float3 dirToLight = (light->GetPosition() - I);
	float dotProduct = max(0.f, dot(normalize(dirToLight), N));
	return light->GetEmission() * light->GetColour() * dotProduct * (1 / PI);
}

float3 Sphere::GetSpecularColour(Light* light, const float3& I, const float3& N, const float3& D)
{
	float3 distToLight = normalize(light->position - I);
	float A = 4 * PI * dot(distToLight, distToLight);
	float3 B = light->GetColour() / A;
	float3 reflected = normalize(reflect(-distToLight, N));
	return pow(-dot(reflected, D), 20.0f);
}

float3 Sphere::GetAlbedo(Light* light, const float3& I, const float3& N, const float3& D)
{
	return material.colour * material.Kd + material.Ks * GetSpecularColour(light, I, N, D);
}
