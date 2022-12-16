#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


// default screen resolution
#define SCRWIDTH	500
#define SCRHEIGHT	500
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
		camPos = float3( 0, -0.7f, -2.0f );
		screenCentre = float3( 0, 0, 0 );
		topLeft = float3( -aspect, 1, 0 );
		topRight = float3( aspect, 1, 0 );
		bottomLeft = float3( -aspect, -1, 0 );
		Translate(0.0, 0.0, -3.2f);
	}
	float getRadians(const float angle)
	{
		return angle * (PI / 180);
	}
	Ray GetPrimaryRay( const int x, const int y )
	{
		// anti-aliasing
		float X = x + RandomFloat() - 0.5;
		float Y = y + RandomFloat() - 0.5;

		// calculate pixel position on virtual screen plane
		const float u = X * (1.0f / SCRWIDTH);
		const float v = Y * (1.0f / SCRHEIGHT);
		const float3 P = topLeft + u * (topRight - topLeft) + v * (bottomLeft - topLeft);
		return Ray( camPos, normalize( P - camPos ) );
	}
	void RotateY(const float angle)
	{
		double c = cos(getRadians(angle));
		double s = sin(getRadians(angle));
		float3 xTransform = float3(c, 0, s);
		float3 zTransform = float3(-s, 0, c);
		float3 vect;
		
		vect = topLeft - camPos;
		topLeft = float3(dot(vect,xTransform), vect[1], dot(vect, zTransform)) + camPos;
		vect = topRight - camPos;
		topRight = float3(dot(vect, xTransform), vect[1], dot(vect, zTransform)) + camPos;
		vect = bottomLeft - camPos;
		bottomLeft = float3(dot(vect, xTransform), vect[1], dot(vect, zTransform)) + camPos;
	}
	void RotateX(const float angle)
	{
		double c = cos(getRadians(angle));
		double s = sin(getRadians(angle));
		float3 yTransform = float3(0, c, -s);
		float3 zTransform = float3(0, s, c);
		float3 vect;

		vect = topLeft - camPos;
		topLeft = float3(vect[0], dot(vect, yTransform), dot(vect, zTransform)) + camPos;
		vect = topRight - camPos;
		topRight = float3(vect[0], dot(vect, yTransform), dot(vect, zTransform)) + camPos;
		vect = bottomLeft - camPos;
		bottomLeft = float3(vect[0], dot(vect, yTransform),  dot(vect, zTransform)) + camPos;
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