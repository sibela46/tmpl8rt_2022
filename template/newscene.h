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
		lightSource = Light(float3(0, 1, 0));
		LoadWalls();
		//LoadBunny();
		LoadSphere();
	}
	void LoadBunny()
	{
		std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
		std::vector<float3> temp_vertices;
		std::vector<float2> temp_uvs;
		std::vector<float3> temp_normals;

		char cCurrentPath[FILENAME_MAX];
		if (!_getcwd(cCurrentPath, sizeof(cCurrentPath)))
		{
			printf("Cannot get current directory!\n");
			return;
		}

		FILE* file = fopen((string(cCurrentPath) + string("\\template\\bunny.obj")).c_str(), "r");
		if (file == NULL) {
			printf("Impossible to open the file!\n");
			return;
		}
		while (true)
		{
			char lineHeader[128];
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF)
				break;
			if (strcmp(lineHeader, "v") == 0)
			{
				float3 vertex;
				fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				temp_vertices.push_back(vertex);
			}
			else if (strcmp(lineHeader, "vn") == 0)
			{
				float3 normal;
				fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
				temp_normals.push_back(normal);
			}
			else if (strcmp(lineHeader, "f") == 0)
			{
				std::string vertex1, vertex2, vertex3;
				unsigned int vertexIndex[3], normalIndex[3];
				int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
				if (matches != 6) {
					printf("File can't be read by our simple parser : ( Try exporting with other options\n");
					return;
				}
				vertexIndices.push_back(vertexIndex[0]);
				vertexIndices.push_back(vertexIndex[1]);
				vertexIndices.push_back(vertexIndex[2]);
				normalIndices.push_back(normalIndex[0]);
				normalIndices.push_back(normalIndex[1]);
				normalIndices.push_back(normalIndex[2]);
			}
		}

		std::vector<float3> out_vertices;
		for (unsigned int i = 0; i < vertexIndices.size(); i++)
		{
			unsigned int vertexIndex = vertexIndices[i];
			float3 vertex = temp_vertices[vertexIndex - 1];
			out_vertices.push_back(vertex * 0.5f);
		}

		std::vector<float3> out_normals;
		for (unsigned int i = 0; i < normalIndices.size(); i++)
		{
			unsigned int normalIndex = normalIndices[i];
			float3 vertex = temp_normals[normalIndex - 1];
			out_normals.push_back(vertex);
		}

		for (unsigned int i = 0; i < out_vertices.size() - 2; i += 3)
		{
			Object triangle = Object(6, out_vertices[i], out_vertices[i + 1], out_vertices[i + 2], 0.f, 0.f, whiteDiffuse, ObjectType::TRIANGLE);
			sceneObjects.push_back(triangle);
		}
	}
	void LoadWalls()
	{
		Object leftWall = Object(1, float3(1, 0, 0), float3(0, 0, 0), float3(0, 0, 0), 0.f, 3.f, redDiffuse, ObjectType::PLANE);
		Object rightWall = Object(2, float3(-1, 0, 0), float3(0, 0, 0), float3(0, 0, 0), 0.f, 2.99f, greenDiffuse, ObjectType::PLANE);
		Object ceiling = Object(3, float3(0, -1, 0), float3(0, 0, 0), float3(0, 0, 0), 0.f, 2.f, whiteDiffuse, ObjectType::PLANE);
		Object floor = Object(4, float3(0, 1, 0), float3(0, 0, 0), float3(0, 0, 0), 0.f, 1.f, whiteDiffuse, ObjectType::PLANE);
		Object backWall = Object(5, float3(0, 0, -1), float3(0, 0, 0), float3(0, 0, 0), 0.f, 3.99f, whiteDiffuse, ObjectType::PLANE);

		sceneObjects.push_back(leftWall);
		sceneObjects.push_back(rightWall);
		sceneObjects.push_back(ceiling);
		sceneObjects.push_back(floor);
		sceneObjects.push_back(backWall);
	}
	void LoadSphere()
	{
		Object sphere = Object(7, float3(0.3f, -0.3f, 0), float3(0, 0, 0), float3(0, 0, 0), 0.3f, 0.f, whiteSpecular, ObjectType::SPHERE);
		sceneObjects.push_back(sphere);

		Object sphere1 = Object(8, float3(-0.3f, -0.3f, 0), float3(0, 0, 0), float3(0, 0, 0), 0.3f, 0.f, mirror, ObjectType::SPHERE);
		sceneObjects.push_back(sphere1);
	}
	float3 GetLightPos() const
	{
		return lightSource.position;
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
	bool IsOccluded( float3 I ) const
	{
		float3 lightDirection = GetLightPos() - I;
		Ray lightRay = Ray(I, normalize(lightDirection));
		FindNearest(lightRay);
		return (lightRay.t*lightRay.t < length(lightDirection));
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
	Material whiteDiffuse = { WHITE,  MaterialType::DIFFUSE, 1.0, 0.0 };
	Material whiteSpecular = { WHITE,  MaterialType::DIFFUSE, 0.5, 1.0 };
	Material redDiffuse = { RED,  MaterialType::DIFFUSE, 1.0, 0.0 };
	Material blueDiffuse = { BLUE,  MaterialType::DIFFUSE, 1.0, 0.0 };
	Material greenDiffuse = { GREEN,  MaterialType::DIFFUSE, 1.0, 0.0 };
	Material mirror = { WHITE,  MaterialType::MIRROR, 1.0, 0.0 };
};

}