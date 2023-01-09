#pragma once

class Light
{
public:
	Light(const float3& p);

	float3	GetPosition();
	float	GetEmission();
	float3	GetColour();

public:
	float3 position;
};
