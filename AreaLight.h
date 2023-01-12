#pragma once

class AreaLight : public Light
{
public:
	AreaLight(const float3& a, const float3& b, const float3& c, const float3& d);
	
	float3	GetPosition(int i) override;
	float3	GetNormal() override;
	float3	GetRandomPoint() override;
	float	GetArea() override;

public:
	std::vector<float3> corners;
	LightType type;
};
