#pragma once

class Scene
{
public:
	Scene();
	virtual ~Scene() = default;

	void FindNearest(Ray& ray);
	bool IsOccluded(float3 I, float3 N);
	float3 GetBeersLaw(Ray& ray);
	float3 GetNormal(int idx, float3 I, float3 D);
	float3 GetShade(int idx, float3 I, float3 D);
	float3 GetAlbedo(int idx, float3 I, float3 N, float3 D);

	void LoadModel(int idx, const char* fileName, Material material, float3 offset);

public:
	std::vector<Object*> sceneObjects;
	Light* light;
};