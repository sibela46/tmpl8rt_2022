#pragma once

class Scene
{
public:
	Scene();
	virtual ~Scene() = default;

	void FindNearest(Ray& ray);
	bool IsOccluded(const float3& I, const float3& N);
	float3 GetBeersLaw(Ray& ray);
	float3 GetNormal(int idx, ObjectType type, const float3& I, const float3& D);
	float3 GetShade(int idx, ObjectType type, const float3& I, const float3& D);
	float3 GetAlbedo(Ray& ray, const float3& N);
	float3 GetSkydomeTexture(const Ray& ray);
	float3 GetSpecularColour(const float3& I, const float3& N, const float3& D);

	void LoadModel(int idx, const char* fileName, Material material, const float3& offset, float scale);

public:
	std::vector<Triangle> triangles;
	std::vector<Bvh> models;
	std::vector<Plane> planes;
	std::vector<Sphere> spheres;
	std::vector<Cube> cubes;
	std::vector<Torus> tori;
	std::vector<Cylinder> cylinders;
	Light* light;
	TextureMap* skydomeTexture;
};