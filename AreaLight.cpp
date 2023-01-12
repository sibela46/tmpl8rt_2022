#include "precomp.h"

AreaLight::AreaLight(const float3& a, const float3& b, const float3& c, const float3& d)
	: Light(a, LightType::AREA)
{
	corners.push_back(a);
	corners.push_back(b);
	corners.push_back(c);
	corners.push_back(d);
}

float3 AreaLight::GetPosition(int i)
{
	return corners[i];
}

float3 AreaLight::GetNormal()
{
	float3 edge1 = corners[1] - corners[0];
	float3 edge2 = corners[2] - corners[0];
	return normalize(cross(edge1, edge2));
}

float3 AreaLight::GetRandomPoint()
{
	float3 edge1 = corners[1] - corners[0];
	float3 edge2 = corners[2] - corners[0];
	return corners[0] + normalize(edge1 * RandomFloat() + edge2 * RandomFloat());
}

float AreaLight::GetArea()
{
	float a = corners[1].x - corners[0].x;
	float b = corners[2].z - corners[0].z;
	return a * b;
}