#pragma once

#include <direct.h>

namespace Tmpl8 {

class NewScene
{
	enum { DIFFUSE = 0, MIRROR = 1, GLASS = 2 };
public:
	NewScene()
	{
		// define all objects here
		lightSource = Light(float3(0, 0, 0), 3.0f);
		LoadWalls();
	}
	void LoadWalls()
	{
		Material whiteDiffuse = { WHITE,  0 };
		Material redDiffuse = { RED,  0 };
		Material blueDiffuse = { BLUE,  0 };
		Material greenDiffuse = { GREEN,  0 };
		Object leftWall = Object(1, float3(1, 0, 0), float3(0, 0, 0), float3(0, 0, 0), 0.f, 3.f, redDiffuse, "plane");
		Object rightWall = Object(2, float3(-1, 0, 0), float3(0, 0, 0), float3(0, 0, 0), 0.f, 2.99f, greenDiffuse, "plane");
		Object ceiling = Object(3, float3(0, -1, 0), float3(0, 0, 0), float3(0, 0, 0), 0.f, 2.f, whiteDiffuse, "plane");
		Object floor = Object(4, float3(0, 1, 0), float3(0, 0, 0), float3(0, 0, 0), 0.f, 1.f, whiteDiffuse, "plane");
		Object backWall = Object(5, float3(0, 0, -1), float3(0, 0, 0), float3(0, 0, 0), 0.f, 3.99f, whiteDiffuse, "plane");

		sceneObjects.push_back(leftWall);
		sceneObjects.push_back(rightWall);
		sceneObjects.push_back(ceiling);
		sceneObjects.push_back(floor);
		sceneObjects.push_back(backWall);
	}
	float3 GetLightPos() const
	{
		
	}
	float3 GetLightColor() const
	{
		return float3( 24, 24, 22 );
	}
	void FindNearest( Ray& ray ) const
	{
		for (int i = 0; i < sceneObjects.size(); ++i)
		{
			sceneObjects[i].Intersect(ray, lightSource);
		}
	}
	bool IsOccluded( Ray& ray ) const
	{
		return false;
	}
	float3 GetNormal( int objIdx, float3 I, float3 wo ) const
	{
		return 0;
	}
	float3 GetAlbedo( int objIdx, float3 I ) const
	{
		return 0;
	}
	float GetReflectivity( int objIdx, float3 I ) const
	{
		if (objIdx == 1 /* ball */) return 1;
		if (objIdx == 6 /* floor */) return 0.3f;
		return 0;
	}
	float GetRefractivity( int objIdx, float3 I ) const
	{
		return objIdx == 3 ? 1.0f : 0.0f;
	}
	__declspec(align(64)) // start a new cacheline here
	std::vector<Object> sceneObjects;
	Light lightSource;
};

}