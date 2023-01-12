#include "precomp.h"

AreaLight::AreaLight(const float3& a, const float3& b, const float3& c, const float3& d)
	: Light(a, LightType::AREA)
{
	corners.push_back(a);
	corners.push_back(b);
	corners.push_back(c);
	corners.push_back(d);

	float x = corners[1].x - corners[0].x;
	float y = corners[2].z - corners[0].z;
	area = x * y;
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

float3 AreaLight::GetRandomPoint(int idx)
{
	float3 edge1 = normalize(corners[1] - corners[0]);
	float3 edge2 = normalize(corners[2] - corners[0]);
	float stratum_x = (idx % 4) * 0.25;
	float stratum_y = (idx / 4) * 0.25;
	float r0 = RandomFloat() * 0.25 + stratum_x;
	float r1 = RandomFloat() * 0.25 + stratum_y;
	return corners[0] + (area * edge1 * r0 + area * edge2 * r1);
}

float AreaLight::GetArea()
{
	return area;
}