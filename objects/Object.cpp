#include "precomp.h"

Object::Object(int index, const float3& position, Material material)
	: index(index), position(position), material(material)
{
}

float3 Object::GetDirectLight(Light* light, const float3& I, const float3& N)
{
	float3 dirToLight = (light->GetPosition() - I);
	float dotProduct = max(0.f, dot(normalize(dirToLight), N));
	return light->GetEmission() * light->GetColour()  * dotProduct * (1/PI);
}

float3 Object::GetIndirectLight(Light* light, const float3& I, const float3& N)
{
	return 0;
}

float3 Object::GetSpecularColour(Light* light, const float3& I, const float3& N, const float3& D)
{
	float3 distToLight = normalize(light->position - I);
	float A = 4 * PI * dot(distToLight, distToLight);
	float3 B = light->GetColour() / A;
	float3 reflected = normalize(reflect(-distToLight, N));
	return pow(-dot(reflected, D), 20.0f);
}

float3 Object::GetAlbedo(Light* light, const float3& I, const float3& N, const float3& D)
{
	return material.colour * material.Kd + material.Ks * GetSpecularColour(light, I, N, D);
}
