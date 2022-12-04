#include "precomp.h"

#define PLANE_X(o,i,m) {if((t=-(ray.O.x+o)*ray.rD.x)<ray.t)ray.t=t,ray.objIdx=i,ray.objMaterial=m;}
#define PLANE_Y(o,i,m) {if((t=-(ray.O.y+o)*ray.rD.y)<ray.t)ray.t=t,ray.objIdx=i,ray.objMaterial=m;}
#define PLANE_Z(o,i,m) {if((t=-(ray.O.z+o)*ray.rD.z)<ray.t)ray.t=t,ray.objIdx=i,ray.objMaterial=m;}

Material whiteDiffuse = { WHITE,  MaterialType::DIFFUSE, 1.0, 0.0 };
Material whiteSpecular = { WHITE,  MaterialType::DIFFUSE, 0.5, 0.5 };
Material redDiffuse = { RED,  MaterialType::DIFFUSE, 1.0, 0.0 };
Material blueDiffuse = { BLUE,  MaterialType::DIFFUSE, 1.0, 0.0 };
Material blueShiny = { BLUE,  MaterialType::DIFFUSE, 1.0, 1.0 };
Material greenDiffuse = { GREEN,  MaterialType::DIFFUSE, 1.0, 0.0 };
Material mirror = { WHITE,  MaterialType::MIRROR, 1.0, 0.0 };
Material glass = { WHITE,  MaterialType::GLASS, 0.0, 1.0 };
Material areaLight = { BRIGHT,  MaterialType::LIGHT, 1.0, 1.0 };

Scene::Scene()
{
	sceneObjects.push_back(new Plane(0, float3(1, 0, 0), 2.f, redDiffuse)); // left wall
	sceneObjects.push_back(new Plane(1, float3(-1, 0, 0), 2.f, greenDiffuse)); // right wall
	sceneObjects.push_back(new Plane(2, float3(0, -1, 0), 1.f, whiteDiffuse)); // floor
	sceneObjects.push_back(new Plane(3, float3(0, 1, 0), 1.f, whiteDiffuse)); // ceiling
	sceneObjects.push_back(new Plane(4, float3(0, 0, 1), 4.f, whiteDiffuse)); // front wall
	sceneObjects.push_back(new Plane(5, float3(0, 0, -1), 2.f, whiteDiffuse)); // back wall
	
	sceneObjects.push_back(new Triangle(6, float3(-0.5f, 0.8f, -1.0f), float3(0.5f, 0.8f, -1.0f), float3(0.5, 0.8f, 0.0f), areaLight));
	sceneObjects.push_back(new Triangle(7, float3(-0.5f, 0.8f, 0.0f), float3(-0.5f, 0.8f, -1.0f), float3(0.5, 0.8f, 0.0f), areaLight));
	
	sceneObjects.push_back(new Sphere(8, float3(-1.0f, -0.5f, 0.f), 0.5f, whiteDiffuse));
	sceneObjects.push_back(new Sphere(9, float3(0.0f, -0.5f, 0.f), 0.5f, redDiffuse));
	sceneObjects.push_back(new Sphere(10, float3(1.0f, -0.5f, 0.f), 0.5f, glass));

	//sceneObjects.push_back(new Triangle(8, float3(3.0f, 0.8f, -3.0f), float3(3.0f, 0.8f, 0.0f), float3(0.0, 0.8f, 0.0f), whiteDiffuse));
	
	//LoadModel(6, "", whiteDiffuse, 0.f);

	//light = new Light(float3(0.f, 0.8f, -2.0f));
}

void Scene::FindNearest(Ray& ray)
{
	float t;
	/*if (ray.D.x < 0) PLANE_X(3, 0, redDiffuse) else PLANE_X(-2.99f, 1, greenDiffuse);
	if (ray.D.y < 0) PLANE_Y(1, 2, whiteDiffuse) else PLANE_Y(-2, 3, whiteDiffuse);
	if (ray.D.z < 0) PLANE_Z(3, 4, whiteDiffuse) else PLANE_Z(-3.99f, 5, whiteDiffuse);*/

	for (int i = 0; i < sceneObjects.size(); ++i)
	{
		sceneObjects[i]->Intersect(ray);
	}
}

bool Scene::IsOccluded(const float3& I, const float3& N)
{
	float3 bias = 0.001f * N;
	float3 dirToLight = (light->position - I);
	Ray rayToLight = Ray(I + bias, normalize(dirToLight));
	FindNearest(rayToLight);

	return rayToLight.t < length(dirToLight);
}

float3 Scene::GetNormal(int idx, const float3& I, const float3& D)
{
	float3 N = sceneObjects[idx]->GetNormal(I);

	if (dot(N, D) > 0) N = -N; // hit backside / inside
	return N;
}

float3 Scene::GetShade(int idx, const float3& I, const float3& N)
{
	if (IsOccluded(I, N)) return 0;
	return sceneObjects[idx]->GetDirectLight(light, I, N) + sceneObjects[idx]->GetIndirectLight(light, I, N);
}

float3 Scene::GetAlbedo(int idx, const float3& I, const float3& N, const float3& D)
{
	return sceneObjects[idx]->GetAlbedo(light, I, N, D);
}

float3 Scene::GetBeersLaw(Ray& ray)
{
	if (ray.objIdx == -1) return 1;
	float distanceTravelled = ray.t;
	float3 absorbance = (ray.objMaterial.colour) * 0.15f * (-distanceTravelled);
	float3 N = sceneObjects[ray.objIdx]->GetNormal(ray.IntersectionPoint());
	bool outside = dot(ray.D, N) > 0;
	if (outside)
	{
		return float3(pow(E, absorbance.x), pow(E, absorbance.y), pow(E, absorbance.z));
	}
	return 1;
}

void Scene::LoadModel(int idx, const char* fileName, Material material, const float3& offset)
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
		Triangle* triangle = new Triangle(idx, out_vertices[i] + offset, out_vertices[i + 1] + offset, out_vertices[i + 2] + offset, material);
		sceneObjects.push_back(triangle);
		idx += 1;
	}
}