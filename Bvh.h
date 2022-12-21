#pragma once
#define BINS 100

__declspec(align(32)) struct BVHNode
{
	float3 aabbMin, aabbMax;
	uint leftFirst, primitivesCount;
	bool isLeaf() { return primitivesCount > 0; }
};

__declspec(align(128)) struct QBVHNode {
	float3 aabbMin[4];
	float3 aabbMax[4];
	int child[4], count[4];
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
		switch (type)
		{
		case ObjectType::TRIANGLE:
			{
				IntersectTriangle(ray);
			}
			break;
		case ObjectType::SPHERE:
			{
				IntersectSphere(ray);
			}
			break;
		case ObjectType::PLANE:
			{
				IntersectPlane(ray);
			}
			break;
		}
	}
	void Intersect(const float3& O, const float3& D, const float distToLight, bool& hitObject)
	{
		switch (type)
		{
		case ObjectType::TRIANGLE:
		{
			IntersectTriangle(O, D, distToLight, hitObject);
		}
		break;
		case ObjectType::SPHERE:
		{
			IntersectSphere(O, D, distToLight, hitObject);
		}
		break;
		case ObjectType::PLANE:
		{
			IntersectPlane(O, D, distToLight, hitObject);
		}
		break;
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
	void IntersectTriangle(const float3& O, const float3& D, const float distToLight, bool& hitObject)
	{
		centroid = (v1 + v2 + v3) * 0.3333f;
		float3 edge1 = v2 - v1;
		float3 edge2 = v3 - v1;
		float3 h = cross(D, edge2);
		float a = dot(edge1, h);
		if (a > -EPSILON && a < EPSILON) return; // the ray is parallel to the triangle	
		float f = 1.0 / a;
		float3 s = O - v1;
		u = f * dot(s, h);
		if (u < 0.0 || u > 1.0) return;
		float3 q = cross(s, edge1);
		v = f * dot(D, q);
		if (v < 0.0 || u + v > 1.0) return;
		float t = f * dot(edge2, q);
		if (t > EPSILON && t < distToLight)
		{
			hitObject = true;
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
	void IntersectPlane(float3 O, float3 D, const float distToLight, bool& hitObject)
	{
		float t = -(dot(O, n) + size) / (dot(D, n));
		if (t < distToLight && t > 0)
		{
			hitObject = true;
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
	void IntersectSphere(float3 O, float3 D, const float distToLight, bool& hitObject)
	{
		float3 oc = O - v1;
		float b = dot(oc, D);
		float c = dot(oc, oc) - size * size;
		float t, d = b * b - c;
		if (d <= 0) return;
		d = sqrtf(d), t = -b - d;
		if (t > 0 && t < distToLight)
		{
			hitObject = true;
			return;
		}
		t = d - b;
		if (t > 0 && t < distToLight)
		{
			hitObject = true;
			return;
		}
	}
};
struct Bin { aabb bounds; int priCount = 0; };

class Bvh
{
public:
	Bvh(vector<Primitive> primitives);
	void	BuildBVH();
	void	CollapseBVH(uint nodeIdx);
	void	UpdateNodeBounds(uint nodeIdx);
	float	CalculateNodeCost(const BVHNode& node);
	void	GetMiddleSplitPosition(BVHNode& node, float& bestPos, int& bestAxis);
	float	FindBestSplitPlane(BVHNode& node, float& splitPos, int& axis);
	void	Subdivide(uint nodeIdx);
	void	ResetNodesUsed();
	void	IntersectBVH(Ray& ray, const uint nodeIdx);
	void	IntersectQBVH(Ray& ray, const uint nodeIdx);
	void	IntersectBVH(const float3& O, const float3& D, const uint nodeIdx, const float distToLight, bool& hitPrimitive);
	bool	IntersectAABB(const Ray& ray, const float3 bmin, const float3 bmax);
	float	IntersectAABB_SSE(const Ray& ray, const __m128& bmin4, const __m128& bmax4);
	bool	IntersectAABB(const float3& O, const float3& D, const float distToLight, const float3 bmin, const float3 bmax);
public:
	static const uint rootNodeIdx = 0;
	uint nodesUsed = 1;
	std::vector<Primitive> primitives;
	int N;
	std::vector<BVHNode> bvhNodes;
	std::vector<QBVHNode> qbvhNodes;
	std::vector<int> primitivesIndices;
};
