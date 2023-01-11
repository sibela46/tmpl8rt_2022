#include "precomp.h"

Light::Light(const float3& position, const LightType type)
	: position(position)
	, type(type)
{

}

void Light::RandomPhotonDirectionAndPosition(float3& pos, float3& dir)
{
	switch (type)
	{
	case LightType::POINT:
		{
			pos = position;
			float x = RandomFloat();
			float y = RandomFloat();
			float z = RandomFloat();
			if (Rand(2.f) > 1.f) x = -x;
			if (Rand(2.f) > 1.f) y = -y;
			if (Rand(2.f) > 1.f) z = -z;
			dir = float3(x, y, z);
		}
		break;
	case LightType::DIRECTIONAL:
		{

		}
		break;
	case LightType::AREA:
		{
			// generate random direction on hemisphere
			float3 N = float3(0, -1, 0);
			float r1 = RandomFloat();
			float r2 = RandomFloat();
			float sinTheta = sqrtf(1 - r1 * r1);
			float phi = 2 * PI * r2;
			float x = sinTheta * cosf(phi);
			float z = sinTheta * sinf(phi);
			float3 randomVec = float3(x, r1, z);
			if (dot(randomVec, N) < 0) randomVec = -randomVec;
			dir = randomVec;

			// generate random position on light
			pos = position;
		}
		break;
	}
}

float3 Light::GetPosition()
{
	return position;
}

float Light::GetEmission()
{
	return 3.0f;
}

float3 Light::GetColour()
{
	return 1.0f;
}

float3 Light::GetNormal()
{
	switch (type)
	{
	case LightType::POINT:
		{
			return float3(0, -1, 0);
		}
		break;
	}
}