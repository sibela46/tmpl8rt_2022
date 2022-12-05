#pragma once

class Light;

class Object
{
public:
	Object(int id, const float3& position, Material material);
	virtual ~Object() = default;

	virtual void	Intersect(Ray& ray) = 0;
	virtual float3	GetNormal(const float3& I) = 0;
	float3			GetAlbedo(Light* light, const float3& I, const float3& N, const float3& D);
	virtual float3	GetDirectLight(Light* light, const float3& I, const float3& N);
	virtual float3	GetSpecularColour(Light* light, const float3& I, const float3& N, const float3& D);

public:
	int index;
	float3 position;
	Material material;
};
