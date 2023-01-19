#pragma once

struct BRDF
{
	float3 specular, diffuse;
	float shininess;
	BRDFType type;
	BRDF(float3 emissive, float3 diffuse, float shininess, BRDFType type)
		: specular(specular), diffuse(diffuse), shininess(shininess), type(type)
	{
	}
	float3 EvalBRDF(const float3& R = 0, const float3& V = 0)
	{
		switch (type)
		{
		case BRDFType::DIFFUSE:
			return diffuse / PI;
		case BRDFType::SPECULAR:
			return specular * pow(dot(R, V), shininess);
		}
	}

	void SampleBRDF()
	{

	}

	void PDF()
	{

	}

	void EvalIndirect()
	{

	}
};