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
		float3 Y = normalize(topLeft - bottomLeft);
		
		yAngle += getRadians(angle);

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
		float3 xTransform = float3(1, 0, 0);
		float3 yTransform = float3(0, c, -s);
		float3 zTransform = float3(0, s, c);
		float3 vect;

		vect = topLeft - camPos;
		vect = RotateY(vect, yAngle);
		float3 res = float3(dot(vect, xTransform), dot(vect, yTransform), dot(vect, zTransform));
		topLeft = RotateY(res, -yAngle) + camPos;
		vect = topRight - camPos;
		vect = RotateY(vect, yAngle);
		res = float3(dot(vect, xTransform), dot(vect, yTransform), dot(vect, zTransform));
		topRight = RotateY(res, -yAngle) + camPos;
		vect = bottomLeft - camPos;
		vect = RotateY(vect, yAngle);
		res = float3(dot(vect, xTransform), dot(vect, yTransform),  dot(vect, zTransform));
		bottomLeft = RotateY(res, -yAngle) + camPos;
	}
	void RotateZ(const float angle)
	{
		double c = cos(getRadians(angle));
		double s = sin(getRadians(angle));
		float3 xTransform = float3(c, -s, 0);
		float3 yTransform = float3(s, c, 0);
		float3 zTransform = float3(0, 0, 1);
		float3 vect;

		vect = topLeft - camPos;
		topLeft = float3(dot(vect, xTransform), dot(vect, yTransform), dot(vect, zTransform)) + camPos;
		vect = topRight - camPos;
		topRight = float3(dot(vect, xTransform), dot(vect, yTransform), dot(vect, zTransform)) + camPos;
		vect = bottomLeft - camPos;
		bottomLeft = float3(dot(vect, xTransform), dot(vect, yTransform),  dot(vect, zTransform)) + camPos;
	}

	float3 RotateY(float3 p, float theta) {
		double c = cos(theta);
		double s = sin(theta);
		float3 res = float3(0.f);

		float3 xTransform = float3(c, 0, -s);
		float3 zTransform = float3(s, 0, c);

		res[0] = dot(p, xTransform);
		res[1] = p[1];
		res[2] = dot(p, zTransform);

		return res;
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
	float yAngle;
};

}