#include "precomp.h"

#define PLANE_X(o,i,m) {if((t=-(ray.O.x+o)*ray.rD.x)<ray.t)ray.t=t,ray.objIdx=i,ray.objMaterial=m;}
#define PLANE_Y(o,i,m) {if((t=-(ray.O.y+o)*ray.rD.y)<ray.t)ray.t=t,ray.objIdx=i,ray.objMaterial=m;}
#define PLANE_Z(o,i,m) {if((t=-(ray.O.z+o)*ray.rD.z)<ray.t)ray.t=t,ray.objIdx=i,ray.objMaterial=m;}

Material whiteDiffuse = { WHITE,  MaterialType::DIFFUSE, 1.0, 0.0 };
Material whiteSpecular = { WHITE,  MaterialType::DIFFUSE, 0.5, 0.5 };
Material redDiffuse = { RED,  MaterialType::DIFFUSE, 1.0, 0.0 };
Material blueDiffuse = { BLUE,  MaterialType::DIFFUSE, 1.0, 0.0 };
Material purpleDiffuse = { PURPLE,  MaterialType::DIFFUSE, 1.0, 0.0 };
Material blueShiny = { BLUE,  MaterialType::DIFFUSE, 1.0, 1.0 };
Material blackShiny = { BLACK,  MaterialType::DIFFUSE, 1.0, 1.0 };
Material greyShiny = { GREY,  MaterialType::DIFFUSE, 1.0, 1.0 };
Material greenDiffuse = { GREEN,  MaterialType::DIFFUSE, 1.0, 0.0 };
Material mirror = { WHITE,  MaterialType::MIRROR, 1.0, 0.0 };
Material glass = { WHITE,  MaterialType::GLASS, 0.0, 1.0 };
Material areaLight = { BRIGHT,  MaterialType::LIGHT, 1.0, 1.0 };

Scene::Scene()
{
	planes.emplace_back(Plane(0, float3(1, 0, 0), 2.f, blueDiffuse)); // left wall
	planes.emplace_back(Plane(1, float3(-1, 0, 0), 2.f, purpleDiffuse)); // right wall
	planes.emplace_back(Plane(2, float3(0, -1, 0), 1.f, whiteDiffuse)); // ceiling
	planes.emplace_back(Plane(3, float3(0, 1, 0), 1.f, whiteDiffuse, new TextureMap("\\assets\\floor.jpg"))); // floor
	planes.emplace_back(Plane(4, float3(0, 0, 1), 4.f, whiteDiffuse)); // front wall
	planes.emplace_back(Plane(5, float3(0, 0, -1), 2.f, whiteDiffuse)); // back wall
	
	spheres.emplace_back(Sphere(0, float3(-0.9f, -0.5f, 0.f), 0.5f, glass, new TextureMap("\\assets\\universe.jpg")));
	//spheres.emplace_back(Sphere(1, float3(-0.3f, -0.5f, 0.f), 0.3f, blueShiny));
	//spheres.emplace_back(Sphere(2, float3(0.3f, -0.5f, 0.f), 0.3f, redDiffuse, new TextureMap("\\assets\\earth.jpg")));
	//spheres.emplace_back(Sphere(3, float3(0.9f, -0.5f, 0.f), 0.3f, glass));
	
	//triangles.emplace_back(Triangle(0, float3(0.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(1.0, 1.0f, 0.0f), blueDiffuse));

	//cubes.emplace_back(Cube(0, float3(0), float3(0.5f), whiteDiffuse, mat4::Translate(1.0f, -0.75f, 0.f)*mat4::RotateY(10)));// , new TextureMap("\\assets\\wood.jpg")));


	skydomeTexture = new TextureMap("\\assets\\sky.jfif");

#ifdef WITTED_STYLE
	light = new Light(float3(0.f, 0.8f, 0.0f));
#else
	triangles.emplace_back(Triangle(0, float3(-0.5f, 0.8f, -1.0f), float3(0.5f, 0.8f, -1.0f), float3(0.5f, 0.8f, 0.0f), areaLight));
	triangles.emplace_back(Triangle(1, float3(-0.5f, 0.8f, 0.0f), float3(-0.5f, 0.8f, -1.0f), float3(0.5f, 0.8f, 0.0f), areaLight));
#endif

	LoadModel(2, "bunny.obj", glass, float3(2.0f, -2.f, 0.0f), 0.5f);
}

void Scene::FindNearest(Ray& ray)
{
	float t;
	/*if (ray.D.x < 0) PLANE_X(3, 0, redDiffuse) else PLANE_X(-2.99f, 1, greenDiffuse);
	if (ray.D.y < 0) PLANE_Y(1, 2, whiteDiffuse) else PLANE_Y(-2, 3, whiteDiffuse);
	if (ray.D.z < 0) PLANE_Z(3, 4, whiteDiffuse) else PLANE_Z(-3.99f, 5, whiteDiffuse);*/

	for (int i = 0; i < triangles.size(); ++i)
	{
		triangles[i].Intersect(ray);
	}
	for (int i = 0; i < spheres.size(); ++i)
	{
		spheres[i].Intersect(ray);
	}
	for (int i = 0; i < cubes.size(); ++i)
	{
		cubes[i].Intersect(ray);
	}
	for (int i = 0; i < tori.size(); ++i)
	{
		tori[i].Intersect(ray);
	}
	for (int i = 0; i < planes.size(); ++i)
	{
		planes[i].Intersect(ray);
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

float3 Scene::GetNormal(int idx, ObjectType type, const float3& I, const float3& D)
{
	float3 N = 0;
	if (type == ObjectType::PLANE)
	{
		N = planes[idx].GetNormal(I);
	}
	else if (type == ObjectType::TRIANGLE)
	{
		N = triangles[idx].GetNormal(I);
	}
	else if (type == ObjectType::SPHERE)
	{
		N = spheres[idx].GetNormal(I);
	}
	else if (type == ObjectType::CUBE)
	{
		N = cubes[idx].GetNormal(I);
	}
	else if (type == ObjectType::TORUS)
	{
		N = tori[idx].GetNormal(I);
	}
	if (dot(N, D) > 0) N = -N; // hit backside / inside
	return N;
}

float3 Scene::GetShade(int idx, ObjectType type, const float3& I, const float3& N)
{
	if (IsOccluded(I, N)) return 0;
	float3 dirToLight = (light->GetPosition() - I);
	float dotProduct = max(0.f, dot(normalize(dirToLight), N));
	return light->GetEmission() * light->GetColour() * dotProduct * (1 / PI);
}

float3 Scene::GetBeersLaw(Ray& ray)
{
	if (ray.objIdx == -1) return 1;
	float distanceTravelled = ray.t;
	float3 absorbance = (ray.objMaterial.colour) * 0.15f * (-distanceTravelled);
	float3 N = GetNormal(ray.objIdx, ray.objType, ray.IntersectionPoint(), ray.D);
	bool outside = dot(ray.D, N) > 0;
	if (outside)
	{
		return float3(pow(E, absorbance.x), pow(E, absorbance.y), pow(E, absorbance.z));
	}
	return 1;
}

void Scene::LoadModel(int idx, const char* fileName, Material material, const float3& offset, float scale)
{
	mat4 transform = mat4::Translate(offset.x, offset.y, offset.z);

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

	FILE* file = fopen((string(cCurrentPath) + string("\\assets\\" + string(fileName))).c_str(), "r");
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
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			char stupidBuffer[1024];
			fgets(stupidBuffer, 1024, file);
			int matches = sscanf(stupidBuffer, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				matches = sscanf(stupidBuffer, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
				if (matches != 6) {
					matches = sscanf(stupidBuffer, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
					if (matches != 3) {
						printf("File can't be read \n");
						fclose(file);
						return ;
					}
				}
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
	}

	std::vector<float4> out_vertices;
	for (unsigned int i = 0; i < vertexIndices.size(); i++)
	{
		unsigned int vertexIndex = vertexIndices[i];
		float4 vertex = float4(temp_vertices[vertexIndex - 1].x, temp_vertices[vertexIndex - 1].y, temp_vertices[vertexIndex - 1].z, 1.f);
		out_vertices.push_back(vertex * transform * scale);
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
		triangles.emplace_back(Triangle(idx, out_vertices[i], out_vertices[i + 1], out_vertices[i + 2], material));
		idx += 1;
	}
}

float3 Scene::GetSpecularColour(const float3& I, const float3& N, const float3& D)
{
#ifdef WITTED_STYLE
	float3 distToLight = normalize(light->position - I);
	float A = 4 * PI * dot(distToLight, distToLight);
	float3 B = light->GetColour() / A;
	float3 reflected = normalize(reflect(-distToLight, N));
	return pow(-dot(reflected, D), 20.0f);
#else
	return 0;
#endif
}

float3 Scene::GetAlbedo(Ray& ray, const float3& N)
{
#ifdef TEXTURING
	return GetTexture(ray, N);
#else
	if (ray.objMaterial.type == MaterialType::GLASS) return ray.objMaterial.colour;
	return ray.objMaterial.colour *ray.objMaterial.Kd + ray.objMaterial.Ks * GetSpecularColour(ray.IntersectionPoint(), N, ray.D);
#endif
}

float3 Scene::GetTexture(Ray& ray, const float3& N)
{
	if (ray.objType == ObjectType::SPHERE) return spheres[ray.objIdx].GetTexture(ray.IntersectionPoint(), N);
	if (ray.objType == ObjectType::PLANE) return planes[ray.objIdx].GetTexture(ray.IntersectionPoint(), N);
	if (ray.objType == ObjectType::CUBE) return cubes[0].GetTexture(ray.IntersectionPoint(), N);
	if (ray.objType == ObjectType::TRIANGLE) return triangles[ray.objIdx].GetTexture(ray.IntersectionPoint(), N);
	return (0, 1, 0);
}

float3 Scene::GetSkydomeTexture(const Ray& ray)
{
	float3 N = ray.D;
	auto phi = atan2(-N.z, N.x) + PI;
	auto theta = acos(-N.y);

	float u = phi / (2 * PI);
	float v = theta / PI;
	return skydomeTexture->GetColourAt(u, v);
}