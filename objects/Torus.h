#pragma once

class Torus
{
public:
	Torus(int id, const float3& p, float r, float d, Material m, TextureMap* t = nullptr);

	void	Intersect(Ray& ray);
	float3	GetNormal(const float3& I);
	float3	GetTexture(const float3& I, const float3& N);
	int SolveQuartic(float c[5], float s[4]);
	int SolveCubic(float c[4], float s[3]);
	int SolveQuadric(float c[3], float s[2]);
	float sign(float x);

public:
	int index = 0;
	float radius = 0;
	float radius2 = 0;
	float3 position; 
	Material material;
	float2 uvCoords;
	TextureMap* texture;
};