#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


// default screen resolution
#define SCRWIDTH	1280
#define SCRHEIGHT	720
// #define FULLSCREEN
// #define DOUBLESIZE

namespace Tmpl8 {

class Camera
{
public:
	Camera()
	{
		// setup a basic view frustum
		focalLen = 1.0f;
		aspect *= focalLen;
		camPos = float3( 0, 0, -2 );
		screenCentre = float3( 0, 0, 0 );
		topLeft = float3( -aspect, 1, 0 );
		topRight = float3( aspect, 1, 0 );
		bottomLeft = float3( -aspect, -1, 0 );

	}
	float getRadians(const float angle)
	{
		return angle * (PI / 180);
	}
	Ray GetPrimaryRay( const int x, const int y )
	{
		// calculate pixel position on virtual screen plane
		const float u = (float)x * (1.0f / SCRWIDTH);
		const float v = (float)y * (1.0f / SCRHEIGHT);
		const float3 P = topLeft + u * (topRight - topLeft) + v * (bottomLeft - topLeft);
		return Ray( camPos, normalize( P - camPos ) );
	}
	void RotateY(const float angle)
	{
		const mat4 rotation = mat4::RotateY(getRadians(angle));
		topLeft = topLeft * rotation;
		topRight = topRight * rotation;
		bottomLeft = bottomLeft * rotation;
		camPos = camPos * rotation;
	}
	void RotateX(const float angle)
	{
		const mat4 rotation = mat4::RotateX(getRadians(angle));
		topLeft = topLeft * rotation;
		topRight = topRight * rotation;
		bottomLeft = bottomLeft * rotation;
		camPos = camPos * rotation;
	}
	void Translate(float3 P)
	{
		float speed = 0.3;
		P = P * speed;
		topLeft += P;
		topRight += P;
		bottomLeft += P;
		camPos += P;
	}

	float aspect = (float)SCRWIDTH / (float)SCRHEIGHT;
	float3 camPos;
	float3 topLeft, topRight, bottomLeft;
	float3 screenCentre;
	float focalLen;
};

}