#pragma once

class Light;

class Object
{
public:
	Object(int id, float3 position, Material material);
	virtual ~Object() = default;

	virtual void	Intersect(Ray& ray) = 0;
	virtual float3	GetNormal(float3 I) = 0;
	virtual float3	GetShade(Light* light, float3 I, float3 N);

public:
	int index;
	float3 position;
	Material material;
};
