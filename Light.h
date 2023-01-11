#pragma once

class Light
{
public:
	Light(const float3& p, const LightType type);

	void	RandomPhotonDirectionAndPosition(float3& pos, float3& dir);

	float3	GetPosition();
	float	GetEmission();
	float3	GetColour();
	float3	GetNormal();

public:
	float3 position;
	LightType type;
};
