#include "precomp.h"

Object::Object(int index, float3 position, Material material)
	: index(index), position(position), material(material)
{
}

float3 Object::GetShade(Light* light, float3 I, float3 N)
{
	float3 dirToLight = (light->GetPosition() - I);
	float dotProduct = max(0.f, dot(normalize(dirToLight), N));
	/*if (dotProduct == 0)
	{
		printf("%f %f %f\n", normalize(dirToLight).x, normalize(dirToLight).y, normalize(dirToLight).z);
		printf("%f %f %f\n", normal.x, normal.y, normal.z);
	}*/
	return light->GetEmission() * light->GetColour()  * dotProduct * (1/PI);
}