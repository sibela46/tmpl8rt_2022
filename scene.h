#pragma once

class Scene
{
public:
	Scene();
	virtual ~Scene() = default;

	void FindNearest(Ray& ray);
	bool IsOccluded(float3 I);
	float3 GetNormal(int idx, float3 I, float3 D);
	float3 GetShade(int idx, float3 I, float3 N);
	float3 GetAlbedo(int idx, float3 I, float3 N, float3 D);

public:
	std::vector<Object*> sceneObjects;
	Light* light;
};