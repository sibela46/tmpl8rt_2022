#include "precomp.h"

Triangle::Triangle(int id, const float4& vertex1, const float4& vertex2, const float4& vertex3, Material m)
{
	v0 = vertex1, v1 = vertex2, v2 = vertex3;
	material = m;
	index = id;
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
		ray.objType = ObjectType::TRIANGLE;
		return;
	}
	return;
}

float3 Triangle::GetNormal(const float3& I)
{
	float3 normal = normalize(cross(v1 - v0, v2 - v0));
	return normal;
}

float3 Triangle::GetDirectLight(Light* light, const float3& I, const float3& N)
{
	float3 dirToLight = (light->GetPosition() - I);
	float dotProduct = max(0.f, dot(normalize(dirToLight), N));
	return light->GetEmission() * light->GetColour() * dotProduct * (1 / PI);
}

float3 Triangle::GetSpecularColour(Light* light, const float3& I, const float3& N, const float3& D)
{
	float3 distToLight = normalize(light->position - I);
	float A = 4 * PI * dot(distToLight, distToLight);
	float3 B = light->GetColour() / A;
	float3 reflected = normalize(reflect(-distToLight, N));
	return pow(-dot(reflected, D), 20.0f);
}

