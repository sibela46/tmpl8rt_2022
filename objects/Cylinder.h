#pragma once

class Cylinder
{
public:
	Cylinder(int id, const float3& p, float r, float d, Material m);

	void	Intersect(Ray& ray);
	float3	GetNormal(const float3& I);
	float3	GetTexture(const float3& I, const float3& N);

public:
	int index = 0;
	float radius = 0;
	float height = 0;
	float3 position; 
	Material material;
};