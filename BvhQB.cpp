#include "precomp.h"

BvhQB::BvhQB(vector<Primitive> pri) {
    primitives = pri;
    
    N = primitives.size();

    primitivesIndices.resize(N);
}

void BvhQB::BuildBVH()
{
    // create the BVH node pool
    bvhNodes = (BVHNodeQB*)_aligned_malloc(sizeof(BVHNodeQB) * N * 2, 64);
    // populate triangle index array
    for (int i = 0; i < N; i++) primitivesIndices[i] = i;
    // calculate triangle centroids for partitioning
    for (int i = 0; i < N; i++)
        primitives[i].CalculateCentroid();
    // assign all triangles to root node
    BVHNodeQB& root = bvhNodes[rootNodeIdx];
    root.leftFirst = 0, root.primitivesCount = N;
    UpdateNodeBounds(rootNodeIdx);
    // subdivide recursively
    Timer t;
    Subdivide(rootNodeIdx);
    printf("BVH (%i nodes) constructed in %.2fms.\n", nodesUsed, t.elapsed() * 1000);
}

void BvhQB::UpdateNodeBounds(uint nodeIdx)
{
    BVHNodeQB& node = bvhNodes[nodeIdx];
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

float BvhQB::FindBestSplitPlane(BVHNodeQB& node, int& axis, float& splitPos)
{
    float bestCost = 1e30f;
    for (int a = 0; a < 3; a++)
    {
        float boundsMin = 1e30f, boundsMax = -1e30f;
        for (uint i = 0; i < node.primitivesCount; i++)
        {
            Primitive& primitive = primitives[primitivesIndices[node.leftFirst + i]];
            boundsMin = min(boundsMin, primitive.centroid[a]);
            boundsMax = max(boundsMax, primitive.centroid[a]);
        }
        if (boundsMin == boundsMax) continue;
        // populate the bins
        Bin bin[BINS];
        float scale = BINS / (boundsMax - boundsMin);
        for (uint i = 0; i < node.primitivesCount; i++)
        {
            Primitive& triangle = primitives[primitivesIndices[node.leftFirst + i]];
            int binIdx = min(BINS - 1, (int)((triangle.centroid[a] - boundsMin) * scale));
            bin[binIdx].priCount++;
            bin[binIdx].bounds.Grow(triangle.v1);
            bin[binIdx].bounds.Grow(triangle.v2);
            bin[binIdx].bounds.Grow(triangle.v3);
        }
        // gather data for the 7 planes between the 8 bins
        float leftArea[BINS - 1], rightArea[BINS - 1];
        int leftCount[BINS - 1], rightCount[BINS - 1];
        aabb leftBox, rightBox;
        int leftSum = 0, rightSum = 0;
        for (int i = 0; i < BINS - 1; i++)
        {
            leftSum += bin[i].priCount;
            leftCount[i] = leftSum;
            leftBox.Grow(bin[i].bounds);
            leftArea[i] = leftBox.Area();
            rightSum += bin[BINS - 1 - i].priCount;
            rightCount[BINS - 2 - i] = rightSum;
            rightBox.Grow(bin[BINS - 1 - i].bounds);
            rightArea[BINS - 2 - i] = rightBox.Area();
        }
        // calculate SAH cost for the 7 planes
        scale = (boundsMax - boundsMin) / BINS;
        for (int i = 0; i < BINS - 1; i++)
        {
            float planeCost = leftCount[i] * leftArea[i] + rightCount[i] * rightArea[i];
            if (planeCost < bestCost)
                axis = a, splitPos = boundsMin + scale * (i + 1), bestCost = planeCost;
        }
    }
    return bestCost;
}

float BvhQB::CalculateNodeCost(BVHNodeQB& node)
{
    float3 e = node.aabbMax - node.aabbMin; // extent of the node
    float surfaceArea = e.x * e.y + e.y * e.z + e.z * e.x;
    return node.primitivesCount * surfaceArea;
}

void BvhQB::Subdivide(uint nodeIdx)
{
    // terminate recursion
    BVHNodeQB& node = bvhNodes[nodeIdx];
    // determine split axis using SAH
    int axis;
    float splitPos;
    float splitCost = FindBestSplitPlane(node, axis, splitPos);
    float nosplitCost = CalculateNodeCost(node);
    if (splitCost >= nosplitCost) return;
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

void BvhQB::IntersectBVH(Ray& ray)
{
    BVHNodeQB* node = &bvhNodes[rootNodeIdx], * stack[64];
    uint stackPtr = 0;
    while (1)
    {
        if (node->isLeaf())
        {
            for (uint i = 0; i < node->primitivesCount; i++)
                primitives[primitivesIndices[node->leftFirst + i]].Intersect(ray);
            if (stackPtr == 0) break; else node = stack[--stackPtr];
            continue;
        }
        BVHNodeQB* child1 = &bvhNodes[node->leftFirst];
        BVHNodeQB* child2 = &bvhNodes[node->leftFirst + 1];
#ifdef USE_SSE
        float dist1 = IntersectAABB_SSE(ray, child1->aabbMin4, child1->aabbMax4);
        float dist2 = IntersectAABB_SSE(ray, child2->aabbMin4, child2->aabbMax4);
#else
        float dist1 = IntersectAABB(ray, child1->aabbMin, child1->aabbMax);
        float dist2 = IntersectAABB(ray, child2->aabbMin, child2->aabbMax);
#endif
        if (dist1 > dist2) { swap(dist1, dist2); swap(child1, child2); }
        if (dist1 == 1e30f)
        {
            if (stackPtr == 0) break; else node = stack[--stackPtr];
        }
        else
        {
            node = child1;
            if (dist2 != 1e30f) stack[stackPtr++] = child2;
        }
    }
}

inline float BvhQB::IntersectAABB(const Ray& ray, const float3 bmin, const float3 bmax)
{
    float tx1 = (bmin.x - ray.O.x) * ray.rD.x, tx2 = (bmax.x - ray.O.x) * ray.rD.x;
    float tmin = min(tx1, tx2), tmax = max(tx1, tx2);
    float ty1 = (bmin.y - ray.O.y) * ray.rD.y, ty2 = (bmax.y - ray.O.y) * ray.rD.y;
    tmin = max(tmin, min(ty1, ty2)), tmax = min(tmax, max(ty1, ty2));
    float tz1 = (bmin.z - ray.O.z) * ray.rD.z, tz2 = (bmax.z - ray.O.z) * ray.rD.z;
    tmin = max(tmin, min(tz1, tz2)), tmax = min(tmax, max(tz1, tz2));
    if (tmax >= tmin && tmin < ray.t && tmax > 0) return tmin; else return 1e30f;
}
float BvhQB::IntersectAABB_SSE(const Ray& ray, const __m128& bmin4, const __m128& bmax4)
{
    static __m128 mask4 = _mm_cmpeq_ps(_mm_setzero_ps(), _mm_set_ps(1, 0, 0, 0));
    __m128 t1 = _mm_mul_ps(_mm_sub_ps(_mm_and_ps(bmin4, mask4), ray.O4), ray.rD4);
    __m128 t2 = _mm_mul_ps(_mm_sub_ps(_mm_and_ps(bmax4, mask4), ray.O4), ray.rD4);
    __m128 vmax4 = _mm_max_ps(t1, t2), vmin4 = _mm_min_ps(t1, t2);
    float tmax = min(vmax4.m128_f32[0], min(vmax4.m128_f32[1], vmax4.m128_f32[2]));
    float tmin = max(vmin4.m128_f32[0], max(vmin4.m128_f32[1], vmin4.m128_f32[2]));
    if (tmax >= tmin && tmin < ray.t && tmax > 0) return tmin; else return 1e30f;
}
