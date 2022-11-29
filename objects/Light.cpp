#include "precomp.h"

Light::Light(float3 position)
	: position(position)
{

}

float3 Light::GetPosition()
{
	return position;
}

float Light::GetEmission()
{
	return 1.0f;
}

float3 Light::GetColour()
{
	return 1.0f;
}