#pragma once

class Cylinder
{
public:
	Cylinder(int id, const float3& p, float r, float d, Material m);

	void	Intersect(Ray& ray);
	float3	GetNormal(const float3& I);
	float3			GetAlbedo(Light* light, const float3& I, const float3& N, const float3& D);
	virtual float3	GetDirectLight(Light* light, const float3& I, const float3& N);
	virtual float3	GetSpecularColour(Light* light, const float3& I, const float3& N, const float3& D);

public:
	int index = 0;
	float radius = 0;
	float height = 0;
	float3 position; 
	Material material;
};