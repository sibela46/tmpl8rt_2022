#pragma once

struct SamplePoint {
    float3 position;
    static const int DIM = 3;

    float operator[](int i) const { return position.cell[i]; }

	SamplePoint(const float3& position)
        : position(position) {}
};

class GuidedSampler
{
public:
	GuidedSampler();

	void AddPath();
	std::vector<SamplePoint> GetSamplePoints();
	float3 SampleDirection(float3 position, float3 N, float& weightIndex);
	void StratifyHemisphereParameters();
	bool RejectIfInRadius(float3 position, float radius);
	void UpdateWeights(float3 position, int weightIndex, float3 flux);
	float3 CosineSampleHemisphere(float r1, float r2, float3 N);
	float3 SampleHemisphere(float r1, float r2, float3 N);
	void AddSamplePoint(float3 position, float3 normal);
	int PickRandomWeightProportional(std::vector<float> sampleWeights);

private:
	std::vector<SamplePoint> samplePoints;
	std::vector<std::vector<float>> weights;
	std::vector<float2> angleStratas;
	KDTree<SamplePoint> sdTree;
	float step;
};