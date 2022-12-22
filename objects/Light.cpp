#include "precomp.h"

Light::Light(const float3& position)
	: position(position)
{

}

float3 Light::GetPosition()
{
	return position;
}

float Light::GetEmission()
{
	return 10.0f;
}

float3 Light::GetColour()
{
	return 1.0f;
}