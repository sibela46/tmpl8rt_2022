#include "precomp.h"

Bvh::Bvh(vector<Primitive> pri, DataCollector* data2) {
    data = data2;
    primitives = pri;
    
    N = primitives.size();
    
    bvhNodes = (BVHNode*)_aligned_malloc(sizeof(BVHNode) * N * 2, 64);

    for (int i = 0; i < 2 * N; i++) {
        QBVHNode newNode = {
            -1e30f, 1e30f, 0, 0
        };
        newNode.child[0] = -1;
        newNode.child[1] = -1;
        newNode.child[2] = -1;
        newNode.child[3] = -1;
        qbvhNodes.push_back(newNode);
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
    Subdivide(rootNodeIdx, 0);
    data->UpdateNodeCount(nodesUsed);
}

void Bvh::CollapseBVH(uint nodeIdx, int depth)
{
    BVHNode& node = bvhNodes[nodeIdx];
    QBVHNode& newNode = qbvhNodes[nodeIdx];

    if (node.isLeaf()) return;

    if (depth > (data->maxTreeDepth)) data->maxTreeDepth = depth;

    nodesUsed++;

    int leftChildIdx = node.leftFirst;
    BVHNode& leftChild = bvhNodes[leftChildIdx];
    int rightChildIdx = node.leftFirst + 1;
    BVHNode& rightChild = bvhNodes[rightChildIdx];

    aabb leftBox, rightBox;
    leftBox.Grow(leftChild.aabbMin);
    leftBox.Grow(leftChild.aabbMax);
    rightBox.Grow(rightChild.aabbMin);
    rightBox.Grow(rightChild.aabbMax);

    int i = 0;
    if (leftChild.isLeaf())
    {
        newNode.aabbMin[i] = leftChild.aabbMin;
        newNode.aabbMax[i] = leftChild.aabbMax;
        newNode.child[i] = leftChild.leftFirst;
        newNode.count[i] = leftChild.primitivesCount;
        i++;
        newNode.aabbMin[i] = leftChild.aabbMin;
        newNode.aabbMax[i] = leftChild.aabbMax;
        newNode.child[i] = leftChild.leftFirst+1;
        newNode.count[i] = leftChild.primitivesCount;
        i++;
    }
    else
    {
        newNode.aabbMin[i] = leftChild.aabbMin;
        newNode.aabbMax[i] = leftChild.aabbMax;
        newNode.child[i] = node.leftFirst;
        newNode.count[i] = leftChild.primitivesCount;
        i++;
    }

    if (rightChild.isLeaf())
    {
        newNode.aabbMin[i] = rightChild.aabbMin;
        newNode.aabbMax[i] = rightChild.aabbMax;
        newNode.child[i] = rightChild.leftFirst;
        newNode.count[i] = rightChild.primitivesCount;
        i++;
        newNode.aabbMin[i] = rightChild.aabbMin;
        newNode.aabbMax[i] = rightChild.aabbMax;
        newNode.child[i] = rightChild.leftFirst + 1;
        newNode.count[i] = rightChild.primitivesCount;
        i++;
    }
    else
    {
        newNode.aabbMin[i] = rightChild.aabbMin;
        newNode.aabbMax[i] = rightChild.aabbMax;
        newNode.child[i] = node.leftFirst+1;
        newNode.count[i] = rightChild.primitivesCount;
        i++;
    }

    /*newNode.aabbxMin4 = _mm_set_ps(newNode.aabbMin[0].x, newNode.aabbMin[1].x, newNode.aabbMin[2].x, newNode.aabbMin[3].x);
    newNode.aabbxMax4 = _mm_set_ps(newNode.aabbMax[0].x, newNode.aabbMax[1].x, newNode.aabbMax[2].x, newNode.aabbMax[3].x);
    newNode.aabbyMin4 = _mm_set_ps(newNode.aabbMin[0].y, newNode.aabbMin[1].y, newNode.aabbMin[2].y, newNode.aabbMin[3].y);
    newNode.aabbyMax4 = _mm_set_ps(newNode.aabbMax[0].y, newNode.aabbMax[1].y, newNode.aabbMax[2].y, newNode.aabbMax[3].y);
    newNode.aabbzMin4 = _mm_set_ps(newNode.aabbMin[0].z, newNode.aabbMin[1].z, newNode.aabbMin[2].z, newNode.aabbMin[3].z);
    newNode.aabbzMax4 = _mm_set_ps(newNode.aabbMax[0].z, newNode.aabbMax[1].z, newNode.aabbMax[2].z, newNode.aabbMax[3].z);*/

    CollapseBVH(node.leftFirst, depth+1);
    CollapseBVH(node.leftFirst + 1, depth+1);
}

void Bvh::UpdateNodeBounds(uint nodeIdx)
{
    BVHNode& node = bvhNodes[nodeIdx];
    node.aabbMin = float3(1e30f);
    node.aabbMax = float3(-1e30f);
    for (uint first = node.leftFirst, i = 0; i < node.primitivesCount; i++)
    {
        int primitiveIdx = primitivesIndices[first + i];
        Primitive& leafPri = primitives[primitiveIdx];
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
                data->UpdateSummedArea(node.aabbMin, node.aabbMax);
            }
            break;
            case ObjectType::SPHERE:
            {
                node.aabbMin = fminf(node.aabbMin, leafPri.v1 - leafPri.size);
                node.aabbMax = fmaxf(node.aabbMax, leafPri.v1 + leafPri.size);
                data->UpdateSummedArea(node.aabbMin, node.aabbMax);
            }
            break;
            case ObjectType::PLANE:
            {
                float3 offset = float3((1 - abs(leafPri.n.x)) * HORIZON, (1 - abs(leafPri.n.y)) * HORIZON, (1 - abs(leafPri.n.z)) * HORIZON);
                node.aabbMin = fminf(node.aabbMin, leafPri.centroid - offset);
                node.aabbMax = fminf(node.aabbMax, leafPri.centroid + offset);
                data->UpdateSummedArea(node.aabbMin, node.aabbMax);
            }
            break;
        }
    }
}

float Bvh::FindBestSplitPlane(BVHNode& node, float& splitPos, int& axis)
{
    float bestCost = 1e30f;
    for (int a = 0; a < 3; a++)
    {
        float boundsMin = 1e30f, boundsMax = -1e30f;
        for (int i = 0; i < node.primitivesCount; i++)
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
            Primitive& primitive = primitives[primitivesIndices[node.leftFirst + i]];
            int binIdx = min(BINS - 1,
                (int)((primitive.centroid[a] - boundsMin) * scale));
            bin[binIdx].priCount++;
            switch (primitive.type)
            {
            case ObjectType::TRIANGLE:
                {
                    bin[binIdx].bounds.Grow(primitive.v1);
                    bin[binIdx].bounds.Grow(primitive.v2);
                    bin[binIdx].bounds.Grow(primitive.v3);
                }
                break;
            case ObjectType::SPHERE:
                {
                    bin[binIdx].bounds.Grow(primitive.v1-primitive.size);
                    bin[binIdx].bounds.Grow(primitive.v1+primitive.size);
                }
                break;
            case ObjectType::PLANE:
                {
                    float3 offset = float3((1 - abs(primitive.n.x)) * HORIZON,
                                           (1 - abs(primitive.n.y)) * HORIZON,
                                           (1 - abs(primitive.n.z)) * HORIZON);
                    bin[binIdx].bounds.Grow(primitive.n + offset);
                    bin[binIdx].bounds.Grow(primitive.n - offset);
                }
                break;
            }
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
            float planeCost =
                leftCount[i] * leftArea[i] + rightCount[i] * rightArea[i];
            if (planeCost < bestCost)
                axis = a, splitPos = boundsMin + scale * (i + 1),
                bestCost = planeCost;
        }
    }
    return bestCost;
}

float Bvh::CalculateNodeCost(const BVHNode& node)
{
    float3 e = node.aabbMax - node.aabbMin; // extent of the node
    float surfaceArea = e.x * e.y + e.y * e.z + e.z * e.x;
    return node.primitivesCount * surfaceArea;
}

void Bvh::GetMiddleSplitPosition(BVHNode& node, float& bestPos, int& bestAxis)
{
    float3 extent = node.aabbMax - node.aabbMin;
    bestAxis = 0;
    if (extent.y > extent.x) bestAxis = 1;
    if (extent.z > extent[bestAxis]) bestAxis = 2;
    bestPos = node.aabbMin[bestAxis] + extent[bestAxis] * 0.5f;
}

void Bvh::Subdivide(uint nodeIdx, int dept)
{
    BVHNode& node = bvhNodes[nodeIdx];
    // determine split axis and position
    int axis;
    float splitPos;
    float splitCost = FindBestSplitPlane(node, splitPos, axis);
    float noSplitCost = CalculateNodeCost(node);
    if (splitCost >= noSplitCost) return;
    if (dept > (data->maxTreeDepth)) data->maxTreeDepth = dept;

    //GetMiddleSplitPosition(node, splitPos, axis);
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
    Subdivide(leftChildIdx, dept + 1);
    Subdivide(rightChildIdx, dept + 1);
}

void Bvh::IntersectQBVH(Ray& ray, const uint nodeIdx)
{
    QBVHNode& node = qbvhNodes[nodeIdx];
    
//    if (!IntersectAABB_SSE(ray, node.aabbxMin4, node.aabbxMax4, node.aabbyMin4, node.aabbyMax4, node.aabbzMin4, node.aabbzMax4)) return;

    for (int c = 0; c < 4; c++)
    {
        if (node.child[c] == -1) continue;
        if (!IntersectAABB(ray, node.aabbMin[c], node.aabbMax[c])) continue;
        if (node.isLeaf(c)) // isLeaf
        {
            for (uint i = 0; i < node.count[c]; i++)
                primitives[primitivesIndices[node.child[c] + i]].Intersect(ray);
        }
        else
        {
            IntersectQBVH(ray, node.child[c]);
        }
    }
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

void Bvh::IntersectBVH(const float3& O, const float3& D, const uint nodeIdx, const float distToLight, bool& hitObject)
{
    if (hitObject) return;
    BVHNode& node = bvhNodes[nodeIdx];
    if (!IntersectAABB(O, D, distToLight, node.aabbMin, node.aabbMax)) return;
    if (node.isLeaf())
    {
        for (uint i = 0; i < node.primitivesCount; i++)
            primitives[primitivesIndices[node.leftFirst + i]].Intersect(O, D, distToLight, hitObject);
    }
    else
    {
        IntersectBVH(O, D, node.leftFirst, distToLight, hitObject);
        IntersectBVH(O, D, node.leftFirst + 1, distToLight, hitObject);
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
    return tmax >= tmin && tmin < ray.t && tmax > 0;
}

float Bvh::IntersectAABB_SSE(const Ray& ray, const __m128& bmin4x, const __m128& bmax4x, const __m128& bmin4y, const __m128& bmax4y, const __m128& bmin4z, const __m128& bmax4z)
{
    static __m128 mask4 = _mm_cmpeq_ps(_mm_setzero_ps(), _mm_set_ps(1, 0, 0, 0));
    __m128 tx1 = _mm_mul_ps(_mm_sub_ps(_mm_and_ps(bmin4x, mask4), ray.Ox4), ray.rDx4);
    __m128 tx2 = _mm_mul_ps(_mm_sub_ps(_mm_and_ps(bmax4x, mask4), ray.Ox4), ray.rDx4);
    __m128 tmin4 = _mm_min_ps(tx1, tx2), tmax4 = _mm_max_ps(tx1, tx2);
    __m128 ty1 = _mm_mul_ps(_mm_sub_ps(_mm_and_ps(bmin4y, mask4), ray.Oy4), ray.rDy4);
    __m128 ty2 = _mm_mul_ps(_mm_sub_ps(_mm_and_ps(bmax4y, mask4), ray.Oy4), ray.rDy4);
    tmin4 = _mm_max_ps(tmin4, _mm_min_ps(ty1, ty2)), tmax4 = _mm_min_ps(tmax4, _mm_max_ps(ty1, ty2));
    __m128 tz1 = _mm_mul_ps(_mm_sub_ps(_mm_and_ps(bmin4z, mask4), ray.Oz4), ray.rDz4);
    __m128 tz2 = _mm_mul_ps(_mm_sub_ps(_mm_and_ps(bmax4z, mask4), ray.Oz4), ray.rDz4);
    tmin4 = _mm_max_ps(tmin4, _mm_min_ps(ty1, ty2)), tmax4 = _mm_min_ps(tmax4, _mm_max_ps(ty1, ty2));
    
    /*__m128 t4 = _mm_set_ps1(1e34f);
    __m128 tmax_zero = _mm_cmpgt_ps(tmax4, _mm_set_ps1(0.f));
    __m128 tmin_t = _mm_cmplt_ps(tmin4, _mm_set_ps1(ray.t));
    __m128 tmax_tmin = _mm_cmpgt_ps(tmin4, tmax4);
    __m128 mask = _mm_and_ps(tmax_zero, _mm_and_ps(tmin_t, tmax_tmin));

    t4 = _mm_blendv_ps(t4, tmin4, mask);*/
    alignas(16) float tmins[4];
    alignas(16) float tmaxs[4];
    _mm_store_ps(tmins, tmin4);
    _mm_store_ps(tmaxs, tmax4);

    for (int i = 0; i < 4; i++)
    {
        if (tmaxs[i] >= tmins[i] && tmins[i] < ray.t && tmaxs[i] > 0) return tmins[i];
    }
    return 1e30f;
}

bool Bvh::IntersectAABB(const float3& O, const float3& D, const float distToLight, const float3 bmin, const float3 bmax)
{
    float tx1 = (bmin.x - O.x) / D.x, tx2 = (bmax.x - O.x) / D.x;
    float tmin = min(tx1, tx2), tmax = max(tx1, tx2);
    float ty1 = (bmin.y - O.y) / D.y, ty2 = (bmax.y - O.y) / D.y;
    tmin = max(tmin, min(ty1, ty2)), tmax = min(tmax, max(ty1, ty2));
    float tz1 = (bmin.z - O.z) / D.z, tz2 = (bmax.z - O.z) / D.z;
    tmin = max(tmin, min(tz1, tz2)), tmax = min(tmax, max(tz1, tz2));
    return tmax >= tmin && tmin < distToLight && tmax > 0;
}
