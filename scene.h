#pragma once

class Scene
{
public:
	Scene(DataCollector* data);
	virtual ~Scene() = default;

	void FindNearest(Ray& ray);
	bool IsOccluded(const Ray& ray);
	bool ShootShadowRay(const float3& O, const float3& D, float distance);
	float3 GetBeersLaw(Ray& ray);
	float3 GetNormal(const Ray& ray);
	float3 GetShade(const Ray& ray);
	float3 GetAlbedo(Ray& ray, const float3& N);
	float3 GetSkydomeTexture(const Ray& ray);
	void RandomPointOnLight(float3& L, float3& NL, float& A);

	float3 GetDirectIllumination(const Ray& ray);
	float3 GetIndirectIllumination();
	float3 GetCausticsIllumination();
	float3 GetRadianceFromPhotonMap(const Ray& ray);

	void BuildPhotonMap();

	void LoadModel(int idx, const char* fileName, Material material, const float3& offset, float scale, float angle);

public:
	Bvh* bvh;
	std::vector<Primitive> planes;
	std::vector<Primitive> primitives;
	Light* light;
	PhotonMap* photonMap;
	TextureMap* skydomeTexture;
	float3 objTranslate;
	DataCollector* data;
};