#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


// default screen resolution
#define SCRWIDTH	640
#define SCRHEIGHT	360
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
		camPos = float3( 0, -0.7f, -2.f );
		screenCentre = float3( 0, 0, 0 );
		topLeft = float3( -aspect, 1, 0 );
		topRight = float3( aspect, 1, 0 );
		bottomLeft = float3( -aspect, -1, 0 );
		Translate(0.0, 0.0, -2.0f);
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
	void Translate(float x, float y, float z)
	{
		float3 X = normalize(topRight - topLeft);
		float3 Y = normalize(topLeft - bottomLeft);
		float3 Z = cross(X, Y);

		float speed = 0.3;
		float3 P = X * x + Y * y + Z * z;
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