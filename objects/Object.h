#pragma once

class Light;

class Object
{
public:
	Object(int id, float3 position, Material material);
	virtual ~Object() = default;

	virtual void	Intersect(Ray& ray) = 0;
	virtual float3	GetNormal(float3 I) = 0;
	float3			GetAlbedo(Light* light, float3 I, float3 N, float3 D);
	virtual float3	GetDirectLight(Light* light, float3 I, float3 N);
	virtual float3	GetIndirectLight(Light* light, float3 I, float3 N);
	virtual float3	GetSpecularColour(Light* light, float3 I, float3 N, float3 D);

public:
	int index;
	float3 position;
	Material material;
};
