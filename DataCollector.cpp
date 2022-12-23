#include "precomp.h"
#include <fstream>

DataCollector::DataCollector() {
    ResetDataCollector();
}

void DataCollector::ResetDataCollector() {
    frameNumber = 0;
    nodeCount = 0;
    summedNodeArea = 0;
    traversalStepsPerIteration = 0;
    intersectedPrimitiveCountPerIteration = 0;
    maxTreeDepth = 0;
    currDepth = 0;
    averagePrimitivePerScreen = 0;
    bvhBuildTime = 0;
    averageFPS = 0;
    averageTraversalStepsPerScreen = 0;
}

void DataCollector::UpdateBuildTime(float bt) {
    bvhBuildTime = bt;
}
void DataCollector::UpdateNodeCount(int nc) {
    nodeCount = nc;
}

void DataCollector::UpdateFPS(float fps) {
    averageFPS += fps;
}

void DataCollector::UpdateSummedArea(float3 aabbMin, float3 aabbMax) {
    float3 diagnal = aabbMax - aabbMin;
    summedNodeArea += (diagnal.x * diagnal.y
        + diagnal.y * diagnal.z
        + diagnal.z * diagnal.x);
}

void DataCollector::UpdateAverageTraversalSteps(int ats) {
    traversalStepsPerIteration += ats;
}

void DataCollector::UpdateFrameNumber() {
    frameNumber += 1;
}

void DataCollector::UpdateIntersectedPrimitives() {
    intersectedPrimitiveCountPerIteration +=1;
}

float DataCollector::GetIntersectedPrimitives() {
    return intersectedPrimitiveCountPerIteration / frameNumber;
}

float DataCollector::GetAverageFPS() {
    return averageFPS / frameNumber;
}

float DataCollector::GetAverageTraversalSteps() {
    return traversalStepsPerIteration / frameNumber;
}

void DataCollector::UpdateTreeDepth(bool isLeaf) {
    currDepth++;
    if (isLeaf) {
        maxTreeDepth = currDepth > maxTreeDepth ? currDepth : maxTreeDepth;
        currDepth = 0;
    }
}

void DataCollector::PrintData(std::string filename) {
    std::ofstream myFile(filename);

    myFile << bvhBuildTime << "\n";
    myFile << GetAverageFPS() << "\n";
    myFile << nodeCount << "\n";
    //myFile << summedNodeArea << "\n";
    //myFile << GetAverageTraversalSteps() << "\n";
    myFile << GetIntersectedPrimitives() << "\n";
    myFile << maxTreeDepth << "\n";

    myFile.close();
}