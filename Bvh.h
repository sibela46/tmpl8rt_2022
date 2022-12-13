#pragma once

class Bvh
{
public:
	Bvh(vector<Triangle> tri);
	struct BVHNode;
	void BuildBVH();
	void UpdateNodeBounds(uint nodeIdx);
	void Subdivide(uint nodeIdx);
	void IntersectBVH(Ray& ray, const uint nodeIdx);
	bool IntersectAABB(const Ray& ray, const float3 bmin, const float3 bmax);

public:
	static const uint rootNodeIdx = 0;
	uint nodesUsed = 1;
	std::vector<Triangle> tri;
	int N;
	std::vector<BVHNode> bvhNodes;
	uint triIdx[];
};
