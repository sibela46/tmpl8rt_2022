#pragma once


class DataCollector
{
public:
	DataCollector();
    void ResetDataCollector();
    void UpdateBuildTime(float bt);
    void UpdateNodeCount(int nc);
    void UpdateFPS(float fps);
    void UpdateSummedArea(float3 aabbMin, float3 aabbMax);
    void UpdateAverageTraversalSteps(int ats);
    void UpdateIntersectedPrimitives();
    void UpdateFrameNumber();
    float GetIntersectedPrimitives();
    float GetAverageFPS();
    float GetAverageTraversalSteps();
    void UpdateTreeDepth(bool isLeaf);
    void PrintData(std::string filename);

public:
    int frameNumber, nodeCount, summedNodeArea, traversalStepsPerIteration, intersectedPrimitiveCountPerIteration, maxTreeDepth, currDepth, averagePrimitivePerScreen, averageFPS, averageTraversalStepsPerScreen;
    float bvhBuildTime;
};