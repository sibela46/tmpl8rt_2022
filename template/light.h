#pragma once

#include <direct.h>

namespace Tmpl8 {

class Light
{
public:
	Light() = default;
	Light(float3 position, float emission)
		: position(position), emission(emission)
	{
	}
	float3 position;
	float emission;
};
}