#include "precomp.h"

GuidedSampler::GuidedSampler()
{
	step = 1.f / NUM_STRATA;
	StratifyHemisphereParameters();
}

void GuidedSampler::AddPath()
{
	sdTree.build(samplePoints);
}

std::vector<SamplePoint> GuidedSampler::GetSamplePoints()
{
	return samplePoints;
}

void GuidedSampler::StratifyHemisphereParameters()
{
	float i = 0;
	float j = 0;
	while (i < (1.0f-step))
	{
		while (j < (1.0f-step))
		{
			angleStratas.push_back(float2(j, i));
			j += step;
		}
		j = 0;
		i += step;
	}
}

void GuidedSampler::AddSamplePoint(float3 position, float3 normal)
{
	SamplePoint newVertex = SamplePoint(position);
	samplePoints.push_back(newVertex);
	std::vector<float> initialWeights(NUM_STRATA * NUM_STRATA);
	for (auto& weight : initialWeights)
	{
		weight = 1.f;
	}
	weights.push_back(initialWeights);
}

float3 GuidedSampler::CosineSampleHemisphere(float param1, float param2, float3 N)
{
	float r0 = param1 + Rand(step), r1 = param2 + Rand(step);
	float r = sqrtf(r0);
	float theta = 2 * PI * r1;
	float x = r * cosf(theta);
	float y = r * sinf(theta);
	return float3(x, y, sqrtf(1 - r0));
}

float3 GuidedSampler::SampleHemisphere(float param1, float param2, float3 N)
{
	float r1 = param1 + Rand(step);
	float r2 = param2 + Rand(step);
	float sinTheta = sqrtf(1 - r1 * r1);
	float phi = 2 * PI * r2;
	float x = sinTheta * cosf(phi);
	float y = sinTheta * sinf(phi);
	float3 randomVec = float3(x, y, r1);
	if (dot(randomVec, N) < 0) randomVec = -randomVec;
	return randomVec;
}

float3 GuidedSampler::SampleDirection(float3 position, float3 normal, float& weightIndex)
{
	int nnIndex = sdTree.nnSearch(SamplePoint(position));
	std::vector<float> samplePointWeights = weights[nnIndex];
	weightIndex = PickRandomWeightProportional(samplePointWeights);
	float3 weightedUnitVec = SampleHemisphere(angleStratas[weightIndex].x, angleStratas[weightIndex].y, normal);
	return weightedUnitVec;
}

void GuidedSampler::UpdateWeights(float3 position, int weightIndex, float3 flux)
{
	int nnIndex = sdTree.nnSearch(SamplePoint(position));
	std::vector<float>& sampleHemisphere = weights[nnIndex];
	sampleHemisphere[weightIndex] = sampleHemisphere[weightIndex] * (1 - LEARNING_RATE) + LEARNING_RATE * (flux.x + flux.y + flux.z) * 0.333f;
}

bool GuidedSampler::RejectIfInRadius(float3 position, float radius)
{
	for (auto point : samplePoints)
	{
		if (length(position - point.position) < radius)
		{
			return true;
		}
	}
	return false;
}

int GuidedSampler::PickRandomWeightProportional(std::vector<float> sampleWeights)
{
	int highestIndex = 0;
	float sum = 0;
	
	for (int i = 0; i < sampleWeights.size(); ++i)
	{
		sum += sampleWeights[i];
		if (RandomFloat() < sampleWeights[i]/sum)
		{
			highestIndex = i;
		}
	}

	return highestIndex;
}