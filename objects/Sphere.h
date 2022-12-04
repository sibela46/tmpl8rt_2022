#pragma once

class Sphere : virtual public Object
{
public:
	Sphere(int id, const float3& p, float r, Material m);

	void	Intersect(Ray& ray) override;
	float3	GetNormal(const float3& I) override;

public:
	float radius = 0;
};