#pragma once
#define BINS 100

__declspec(align(32)) struct BVHNode
{
	float3 aabbMin, aabbMax;
	uint leftFirst, triCount;
	bool isLeaf() { return triCount > 0; }
};

struct Bin { aabb bounds; int triCount = 0; };

class Bvh
{
public:
	Bvh(vector<Triangle> tri);
	void	BuildBVH();
	void	UpdateNodeBounds(uint nodeIdx);
	void	Subdivide(uint nodeIdx);
	void	IntersectBVH(Ray& ray, const uint nodeIdx);
	bool	IntersectAABB(const Ray& ray, const float3 bmin, const float3 bmax);
	float	GetNormal(int idx, ObjectType type, const float3& I, const float3& D);
public:
	static const uint rootNodeIdx = 0;
	uint nodesUsed = 1;
	std::vector<Triangle> tri;
	int N;
	std::vector<BVHNode> bvhNodes;
	std::vector<int> triIndices;
};
