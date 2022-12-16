#include "precomp.h"

Bvh::Bvh(vector<Triangle> triangles) {
    tri = triangles;
    
    N = tri.size();
    
    for (int i = 0; i < 2 * N; i++) {
        BVHNode newNode = {
            float3(1e30f), float3(-1e30f), 0, 0
        };
        bvhNodes.push_back(newNode);
    }
    
    triIndices.resize(N);
}

void Bvh::BuildBVH()
{
    // populate triangle index array
    for (int i = 0; i < N; i++) triIndices[i] = i;
    // calculate triangle centroids for partitioning
    for (int i = 0; i < N; i++)
        tri[i].centroid = (tri[i].v0 + tri[i].v1 + tri[i].v2) * 0.3333f;
    // assign all triangles to root node
    BVHNode& root = bvhNodes[rootNodeIdx];
    root.leftFirst = 0, root.triCount = N;
    UpdateNodeBounds(rootNodeIdx);
    // subdivide recursively
    Subdivide(rootNodeIdx);
}

void Bvh::UpdateNodeBounds(uint nodeIdx)
{
    BVHNode& node = bvhNodes[nodeIdx];
    node.aabbMin = float3(1e30f);
    node.aabbMax = float3(-1e30f);
    for (uint first = node.leftFirst, i = 0; i < node.triCount; i++)
    {
        Triangle& leafTri = tri[first + i];
        node.aabbMin = fminf(node.aabbMin, leafTri.v0);
        node.aabbMin = fminf(node.aabbMin, leafTri.v1);
        node.aabbMin = fminf(node.aabbMin, leafTri.v2);
        node.aabbMax = fmaxf(node.aabbMax, leafTri.v0);
        node.aabbMax = fmaxf(node.aabbMax, leafTri.v1);
        node.aabbMax = fmaxf(node.aabbMax, leafTri.v2);
    }
}

void Bvh::Subdivide(uint nodeIdx)
{
    // terminate recursion
    BVHNode& node = bvhNodes[nodeIdx];
    if (node.triCount <= 2) return;
    // determine split axis and position
    float3 extent = node.aabbMax - node.aabbMin;
    int axis = 0;
    if (extent.y > extent.x) axis = 1;
    if (extent.z > extent[axis]) axis = 2;
    float splitPos = node.aabbMin[axis] + extent[axis] * 0.5f;
    // in-place partition
    int i = node.leftFirst;
    int j = i + node.triCount - 1;
    while (i <= j)
    {
        if (tri[triIndices[i]].centroid[axis] < splitPos)
            i++;
        else
            swap(triIndices[i], triIndices[j--]);
    }
    // abort split if one of the sides is empty
    int leftCount = i - node.leftFirst;
    if (leftCount == 0 || leftCount == node.triCount) return;
    // create child nodes
    int leftChildIdx = nodesUsed++;
    int rightChildIdx = nodesUsed++;
    bvhNodes[leftChildIdx].leftFirst = node.leftFirst;
    bvhNodes[leftChildIdx].triCount = leftCount;
    bvhNodes[rightChildIdx].leftFirst = i;
    bvhNodes[rightChildIdx].triCount = node.triCount - leftCount;
    node.leftFirst = leftChildIdx;
    node.triCount = 0;
    UpdateNodeBounds(leftChildIdx);
    UpdateNodeBounds(rightChildIdx);
    // recurse
    Subdivide(leftChildIdx);
    Subdivide(rightChildIdx);
}


void Bvh::IntersectBVH(Ray& ray, const uint nodeIdx)
{
    BVHNode& node = bvhNodes[nodeIdx];
    if (!IntersectAABB(ray, node.aabbMin, node.aabbMax)) return;
    if (node.isLeaf())
    {
        for (uint i = 0; i < node.triCount; i++)
            tri[triIndices[node.leftFirst + i]].Intersect(ray);
    }
    else
    {
        IntersectBVH(ray, node.leftFirst);
        IntersectBVH(ray, node.leftFirst + 1);
    }
}

bool Bvh::IntersectAABB(const Ray& ray, const float3 bmin, const float3 bmax)
{
    float tx1 = (bmin.x - ray.O.x) / ray.D.x, tx2 = (bmax.x - ray.O.x) / ray.D.x;
    float tmin = min(tx1, tx2), tmax = max(tx1, tx2);
    float ty1 = (bmin.y - ray.O.y) / ray.D.y, ty2 = (bmax.y - ray.O.y) / ray.D.y;
    tmin = max(tmin, min(ty1, ty2)), tmax = min(tmax, max(ty1, ty2));
    float tz1 = (bmin.z - ray.O.z) / ray.D.z, tz2 = (bmax.z - ray.O.z) / ray.D.z;
    tmin = max(tmin, min(tz1, tz2)), tmax = min(tmax, max(tz1, tz2));
    return tmax >= tmin && tmin < ray.t&& tmax > 0;
}
