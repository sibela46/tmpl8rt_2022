#pragma once

class Sphere
{
public:
	Sphere(int id, const float3& p, float r, Material m, TextureMap* t = nullptr);

	void	Intersect(Ray& ray);
	float3	GetNormal(const float3& I);
	float3	GetTexture(const float3& I, const float3& N);

public:
	int index = 0;
	float radius = 0;
	float3 position;
	Material material;
	float2 uvCoords;
	TextureMap* texture;
};