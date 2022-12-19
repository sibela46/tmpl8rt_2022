#pragma once
#define BINS 100

__declspec(align(32)) struct BVHNode
{
	float3 aabbMin, aabbMax;
	uint leftFirst, primitivesCount;
	bool isLeaf() { return primitivesCount > 0; }
};

struct Primitive { 
	int index; ObjectType type; float3 centroid; float3 v1; float3 v2; float3 v3; float3 n; float size; float u; float v; Material material;
	void CalculateCentroid()
	{
		switch (type)
		{
			case ObjectType::TRIANGLE:
			{
				centroid = (v1 + v2 + v3) * 0.3333f;
			}
			break;
			case ObjectType::SPHERE:
			{
				centroid = v1;
			}
			break;
			case ObjectType::PLANE:
			{
				centroid = n * size;
			}
			break;
		}
	}
	void Intersect(Ray& ray)
	{
		if (type == ObjectType::TRIANGLE)
		{
			IntersectTriangle(ray);
		}
		else if (type == ObjectType::PLANE)
		{
			IntersectPlane(ray);
		}
		else if (type == ObjectType::SPHERE)
		{
			IntersectSphere(ray);
		}
	}
	void IntersectTriangle(Ray& ray)
	{
		centroid = (v1 + v2 + v3) * 0.3333f;
		float3 edge1 = v2 - v1;
		float3 edge2 = v3 - v1;
		float3 h = cross(ray.D, edge2);
		float a = dot(edge1, h);
		if (a > -EPSILON && a < EPSILON) return; // the ray is parallel to the triangle	
		float f = 1.0 / a;
		float3 s = ray.O - v1;
		u = f * dot(s, h);
		if (u < 0.0 || u > 1.0) return;
		float3 q = cross(s, edge1);
		v = f * dot(ray.D, q);
		if (v < 0.0 || u + v > 1.0) return;
		float t = f * dot(edge2, q);
		if (t > EPSILON && t < ray.t)
		{
			ray.t = t;
			ray.objIdx = index;
			ray.objMaterial = material;
			ray.objType = ObjectType::TRIANGLE;
			ray.normal = n;
			ray.inside = dot(ray.D, n) > 0;
			return;
		}
		return;
	}
	void IntersectPlane(Ray& ray)
	{
		float t = -(dot(ray.O, n) + size) / (dot(ray.D, n));
		if (t < ray.t && t > 0)
		{
			ray.t = t;
			ray.objIdx = index;
			ray.objMaterial = material;
			ray.objType = ObjectType::PLANE;
			ray.normal = n;
			ray.inside = dot(ray.D, n) > 0;
		}
	}
	void IntersectSphere(Ray& ray)
	{
		float3 oc = ray.O - v1;
		float b = dot(oc, ray.D);
		float c = dot(oc, oc) - size * size;
		float t, d = b * b - c;
		if (d <= 0) return;
		d = sqrtf(d), t = -b - d;
		if (t < ray.t && t > 0)
		{
			float3 I = ray.O + t * ray.D;
			n = normalize(I - v1);
			u = 0.5 - atan2(n.z, n.x) * (1 / PI);
			v = 0.5 - asin(n.y) * (1 / PI);
			ray.t = t, ray.objIdx = index, ray.objMaterial = material, ray.objType = ObjectType::SPHERE, ray.normal = n, ray.inside = dot(ray.D, n) > 0;
			return;
		}
		t = d - b;
		if (t < ray.t && t > 0)
		{
			float3 I = ray.O + t * ray.D;
			n = normalize(I - v1);
			u = 0.5 - atan2(n.z, n.x) * (1 / PI);
			v = 0.5 - asin(n.y) * (1 / PI);
			ray.t = t, ray.objIdx = index, ray.objMaterial = material, ray.objType = ObjectType::SPHERE, ray.normal = n, ray.inside = dot(ray.D, n) > 0;
			return;
		}
	}
};
struct Bin { aabb bounds; int triCount = 0; };

class Bvh
{
public:
	Bvh(vector<Primitive> primitives);
	void	BuildBVH();
	void	UpdateNodeBounds(uint nodeIdx);
	void	Subdivide(uint nodeIdx);
	void	IntersectBVH(Ray& ray, const uint nodeIdx);
	bool	IntersectAABB(const Ray& ray, const float3 bmin, const float3 bmax);
public:
	static const uint rootNodeIdx = 0;
	uint nodesUsed = 1;
	std::vector<Primitive> primitives;
	int N;
	std::vector<BVHNode> bvhNodes;
	std::vector<int> primitivesIndices;
};
