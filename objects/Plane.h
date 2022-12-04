#pragma once

class Plane : virtual public Object
{
public:
	Plane(int id, const float3& p, float r, Material m);

	void	Intersect(Ray& ray) override;
	float3	GetNormal(const float3& I) override;
public:
	float distance = 0;
	float3 normal = 0;
};