#pragma once

class Light
{
public:
	Light(const float3& p, const LightType type);

	void	RandomPhotonDirectionAndPosition(float3& pos, float3& dir);

	virtual float3	GetPosition(int i = 0);
	virtual float3	GetNormal();
	virtual float3	GetRandomPoint(int idx = 0);
	virtual float	GetArea();
	float	GetEmission();
	float3	GetColour();

public:
	float3 position;
	LightType type;
};
