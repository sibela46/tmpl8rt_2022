#pragma once
#define BINS 100
#define USE_SSE

struct BVHNodeQB
{
	union { struct { float3 aabbMin; uint leftFirst; }; __m128 aabbMin4; };
	union { struct { float3 aabbMax; uint primitivesCount; }; __m128 aabbMax4; };
	bool isLeaf() { return primitivesCount > 0; }
};

class BvhQB
{
public:
	BvhQB(vector<Primitive> primitives);
	void	BuildBVH();
	void	UpdateNodeBounds(uint nodeIdx);
	void	Subdivide(uint nodeIdx);
	void	IntersectBVH(Ray& ray);
	inline float IntersectAABB(const Ray& ray, const float3 bmin, const float3 bmax);
	float IntersectAABB_SSE(const Ray& ray, const __m128& bmin4, const __m128& bmax4);
	float FindBestSplitPlane(BVHNodeQB& node, int& axis, float& splitPos);
	float CalculateNodeCost(BVHNodeQB& node);

public:
	static const uint rootNodeIdx = 0;
	uint nodesUsed = 1;
	std::vector<Primitive> primitives;
	int N;
	BVHNodeQB* bvhNodes = 0;
	std::vector<int> primitivesIndices;
};
