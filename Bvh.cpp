#include "precomp.h"

Bvh::Bvh(vector<Primitive> pri) {
    primitives = pri;
    
    N = primitives.size();
    
    for (int i = 0; i < 2 * N; i++) {
        BVHNode newNode = {
            float3(1e30f), float3(-1e30f), 0, 0
        };
        bvhNodes.push_back(newNode);
    }
    
    primitivesIndices.resize(N);
}

void Bvh::BuildBVH()
{
    // populate triangle index array
    for (int i = 0; i < N; i++) primitivesIndices[i] = i;
    // calculate centroid
    for (int i = 0; i < N; i++) primitives[i].CalculateCentroid();
    // assign all triangles to root node
    BVHNode& root = bvhNodes[rootNodeIdx];
    root.leftFirst = 0, root.primitivesCount = N;
    UpdateNodeBounds(rootNodeIdx);
    // subdivide recursively
    Subdivide(rootNodeIdx);
}

void Bvh::UpdateNodeBounds(uint nodeIdx)
{
    BVHNode& node = bvhNodes[nodeIdx];
    node.aabbMin = float3(1e30f);
    node.aabbMax = float3(-1e30f);
    for (uint first = node.leftFirst, i = 0; i < node.primitivesCount; i++)
    {
        Primitive& leafPri = primitives[first + i];
        switch (leafPri.type)
        {
            case ObjectType::TRIANGLE:
            {
                node.aabbMin = fminf(node.aabbMin, leafPri.v1);
                node.aabbMin = fminf(node.aabbMin, leafPri.v2);
                node.aabbMin = fminf(node.aabbMin, leafPri.v3);
                node.aabbMax = fmaxf(node.aabbMax, leafPri.v1);
                node.aabbMax = fmaxf(node.aabbMax, leafPri.v2);
                node.aabbMax = fmaxf(node.aabbMax, leafPri.v3);
            }
            break;
            case ObjectType::SPHERE:
            {
                node.aabbMin = leafPri.v1 - leafPri.size;
                node.aabbMax = leafPri.v1 + leafPri.size;
            }
            break;
            case ObjectType::PLANE:
            {
                float3 offset = float3((1 - abs(leafPri.n.x)) * HORIZON, (1 - abs(leafPri.n.y)) * HORIZON, (1 - abs(leafPri.n.z)) * HORIZON);
                printf("id: %d, center: %f %f %f\n", leafPri.index, leafPri.centroid.x, leafPri.centroid.y, leafPri.centroid.z);
                printf("id: %d, offset: %f %f %f\n", leafPri.index, offset.x, offset.y, offset.z);
                node.aabbMin = leafPri.centroid - offset;
                node.aabbMax = leafPri.centroid + offset;
                printf("id: %d, nodeMin: %f %f %f\n", leafPri.index, node.aabbMin.x, node.aabbMin.y, node.aabbMin.z);
                printf("id: %d, nodeMax: %f %f %f\n", leafPri.index, node.aabbMax.x, node.aabbMax.y, node.aabbMax.z);
            }
            break;
        }
    }
}

void Bvh::GetSAHSplitPosition(uint nodeIdx, float& bestCost, float& bestPos, int& bestAxis)
{
    // determine split axis using SAH
    BVHNode& node = bvhNodes[nodeIdx];
    for (int axis = 0; axis < 3; axis++) for (uint i = 0; i < node.primitivesCount; i++)
    {
        Primitive& triangle = primitives[primitivesIndices[node.leftFirst + i]];
        float candidatePos = triangle.centroid[axis];
        float cost = EvaluateSAH(node, axis, candidatePos);
        if (cost < bestCost)
            bestPos = candidatePos, bestAxis = axis, bestCost = cost;
    }
}

void Bvh::GetMiddleSplitPosition(uint nodeIdx, float& bestPos, int& bestAxis)
{
    BVHNode& node = bvhNodes[nodeIdx];
    float3 extent = node.aabbMax - node.aabbMin;
    bestAxis = 0;
    if (extent.y > extent.x) bestAxis = 1;
    if (extent.z > extent[bestAxis]) bestAxis = 2;
    bestPos = node.aabbMin[bestAxis] + extent[bestAxis] * 0.5f;
}

float Bvh::EvaluateSAH(BVHNode& node, int axis, float pos)
{
    // determine triangle counts and bounds for this split candidate
    aabb leftBox, rightBox;
    int leftCount = 0, rightCount = 0;
    for (uint i = 0; i < node.primitivesCount; i++)
    {
        Primitive& triangle = primitives[primitivesIndices[node.leftFirst + i]];
        if (triangle.centroid[axis] < pos)
        {
            leftCount++;
            leftBox.Grow(triangle.v1);
            leftBox.Grow(triangle.v2);
            leftBox.Grow(triangle.v3);
        }
        else
        {
            rightCount++;
            rightBox.Grow(triangle.v1);
            rightBox.Grow(triangle.v2);
            rightBox.Grow(triangle.v3);
        }
    }
    float cost = leftCount * leftBox.Area() + rightCount * rightBox.Area();
    return cost > 0 ? cost : 1e30f;
}

void Bvh::Subdivide(uint nodeIdx)
{
    // terminate recursion
    BVHNode& node = bvhNodes[nodeIdx];
    if (node.primitivesCount <= 2) return;
    // determine split axis and position
    int axis = -1;
    float splitPos = 0.f, bestCost = 1e30f;
    GetMiddleSplitPosition(nodeIdx, splitPos, axis);
    //GetSAHSplitPosition(nodeIdx, bestCost, splitPos, axis);
    //// terminate if more expensive than parent
    //float3 e = node.aabbMax - node.aabbMin; // extent of parent
    //float parentArea = e.x * e.y + e.y * e.z + e.z * e.x;
    //float parentCost = node.primitivesCount * parentArea;
    //if (bestCost >= parentCost) return;
    // in-place partition
    int i = node.leftFirst;
    int j = i + node.primitivesCount - 1;
    while (i <= j)
    {
        if (primitives[primitivesIndices[i]].centroid[axis] < splitPos)
            i++;
        else
            swap(primitivesIndices[i], primitivesIndices[j--]);
    }
    // abort split if one of the sides is empty
    int leftCount = i - node.leftFirst;
    if (leftCount == 0 || leftCount == node.primitivesCount) return;
    // create child nodes
    int leftChildIdx = nodesUsed++;
    int rightChildIdx = nodesUsed++;
    bvhNodes[leftChildIdx].leftFirst = node.leftFirst;
    bvhNodes[leftChildIdx].primitivesCount = leftCount;
    bvhNodes[rightChildIdx].leftFirst = i;
    bvhNodes[rightChildIdx].primitivesCount = node.primitivesCount - leftCount;
    node.leftFirst = leftChildIdx;
    node.primitivesCount = 0;
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
        for (uint i = 0; i < node.primitivesCount; i++)
            primitives[primitivesIndices[node.leftFirst + i]].Intersect(ray);
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
