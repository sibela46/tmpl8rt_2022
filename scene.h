#pragma once

class Scene
{
public:
	Scene();
	virtual ~Scene() = default;

	void FindNearest(Ray& ray);
	bool IsOccluded(const float3& I, const float3& N);
	float3 GetBeersLaw(Ray& ray);
	float3 GetNormal(int idx, const float3& I, const float3& D);
	float3 GetShade(int idx, const float3& I, const float3& D);
	float3 GetAlbedo(int idx, const float3& I, const float3& N, const float3& D);

	void LoadModel(int idx, const char* fileName, Material material, const float3& offset);

public:
	std::vector<Object*> sceneObjects;
	Light* light;
};