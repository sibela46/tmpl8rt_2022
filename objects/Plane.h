#pragma once

class Plane
{
public:
	Plane(int id, const float3& p, float r, Material m);

	void	Intersect(Ray& ray);
	float3	GetNormal(const float3& I);
	float3	GetUVCoords();
	float3	GetTexture(const float3& I, const float3& N);

public:
	float distance = 0;
	float3 normal = 0;
	int index = 0;
	Material material;
	TextureMap* texture;
};