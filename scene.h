#pragma once

class Scene
{
public:
	Scene(DataCollector* data);
	virtual ~Scene() = default;

	void FindNearest(Ray& ray);
	bool IsOccluded(const Ray& ray);
	float3 GetBeersLaw(Ray& ray);
	float3 GetNormal(const Ray& ray);
	float3 GetShade(const Ray& ray);
	float3 GetAlbedo(Ray& ray, const float3& N);
	float3 GetSkydomeTexture(const Ray& ray);
	void SetObjTranslate(float3 pos);

	void LoadModel(int idx, const char* fileName, Material material, const float3& offset, float scale, float angle);

public:
	Bvh* bvh;
	std::vector<Primitive> planes;
	std::vector<Primitive> primitives;
	Light* light;
	TextureMap* skydomeTexture;
	float3 objTranslate;
	DataCollector* data;
};