#pragma once

#include <direct.h>

namespace Tmpl8 {

class Light
{
public:
	Light() = default;
	Light(float3 position)
		: position(position)
	{
		emission = 1.f;
		colour = float3(1, 1, 1);
	}
	void Intersect()
	{

	}
	float3 position;
	float3 colour;
	float emission;
};
}