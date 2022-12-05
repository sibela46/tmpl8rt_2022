#pragma once

class Plane
{
public:
	Plane(int id, const float3& p, float r, Material m);

	void	Intersect(Ray& ray);
	float3	GetNormal(const float3& I);
	float3			GetAlbedo(Light* light, const float3& I, const float3& N, const float3& D);
	virtual float3	GetDirectLight(Light* light, const float3& I, const float3& N);
	virtual float3	GetSpecularColour(Light* light, const float3& I, const float3& N, const float3& D);
public:
	float distance = 0;
	float3 normal = 0;
	int index = 0;
	Material material;
};