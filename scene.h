#pragma once

class Scene
{
public:
	Scene();
	virtual ~Scene() = default;

	void FindNearest(Ray& ray);
	bool IsOccluded(const Ray& ray);
	float3 GetBeersLaw(Ray& ray);
	float3 GetNormal(int idx, ObjectType type, const float3& I, const float3& D);
	float3 GetNormal(const Ray& ray);
	float3 GetShade(const Ray& ray);
	float3 GetAlbedo(Ray& ray, const float3& N);
	float3 GetSkydomeTexture(const Ray& ray);

	void LoadModel(int idx, const char* fileName, Material material, const float3& offset, float scale);
	void LoadModelNew(int idx, const char* fileName, Material material, const float3& offset, float scale, float angle);

public:
	std::vector<Triangle> triangles;
	Bvh* bvh;
	std::vector<Plane> planes;
	std::vector<Sphere> spheres;
	std::vector<Cube> cubes;
	std::vector<Torus> tori;
	std::vector<Cylinder> cylinders;
	std::vector<Primitive> primitives;
	Light* light;
	TextureMap* skydomeTexture;
};