#pragma once
#define BINS 100

struct BVHNode
{
	union { struct { float3 aabbMin; uint leftFirst; }; __m128 aabbMin4; };
	union { struct { float3 aabbMax; uint primitivesCount; }; __m128 aabbMax4; };
	bool isLeaf() { return primitivesCount > 0; }
};

__declspec(align(128)) struct QBVHNode {
	float3 aabbMin[4];
	float3 aabbMax[4];
	int child[4], count[4];
	bool isLeaf(int i) { return count[i] > 0; }
};

struct Bin { aabb bounds; int priCount = 0; };

class Bvh
{
public:
	Bvh(vector<Primitive> primitives, DataCollector* data);
	void	BuildBVH();
	void    ResetNodesUsed() { nodesUsed = 0; };
	uint	GetNodesUsed() { return nodesUsed; };
	void	CollapseBVH(uint nodeIdx, int depth);
	void	UpdateNodeBounds(uint nodeIdx);
	float	CalculateNodeCost(const BVHNode& node);
	void	GetMiddleSplitPosition(BVHNode& node, float& bestPos, int& bestAxis);
	float	FindBestSplitPlane(BVHNode& node, float& splitPos, int& axis);
	void	Subdivide(uint nodeIdx, int dept);
	void	IntersectBVH(Ray& ray, const uint nodeIdx);
	void	IntersectQBVH(Ray& ray, const uint nodeIdx);
	void	IntersectBVH(const float3& O, const float3& D, const uint nodeIdx, const float distToLight, bool& hitPrimitive);
	bool	IntersectAABB(const Ray& ray, const float3 bmin, const float3 bmax);
	float	IntersectAABB_SSE(const Ray& ray, const __m128& bmin4x, const __m128& bmax4x, const __m128& bmin4y, const __m128& bmax4y, const __m128& bmin4z, const __m128& bmax4z);
	bool	IntersectAABB(const float3& O, const float3& D, const float distToLight, const float3 bmin, const float3 bmax);
public:
	static const uint rootNodeIdx = 0;
	uint nodesUsed = 1;
	std::vector<Primitive> primitives;
	int N;
	BVHNode* bvhNodes;
	std::vector<QBVHNode> qbvhNodes;
	std::vector<int> primitivesIndices;
	DataCollector* data;
};
