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
	Texture* texture = new Texture();
	planes.emplace_back(Plane(0, float3(1, 0, 0), 2.f, redDiffuse)); // left wall
	planes.emplace_back(Plane(1, float3(-1, 0, 0), 2.f, greenDiffuse)); // right wall
	//planes.emplace_back(Plane(2, float3(0, -1, 0), 1.f, whiteDiffuse)); // ceiling
	planes.emplace_back(Plane(2, float3(0, 1, 0), 1.f, whiteDiffuse)); // floor
	planes.emplace_back(Plane(3, float3(0, 0, 1), 4.f, whiteDiffuse)); // front wall
	planes.emplace_back(Plane(4, float3(0, 0, -1), 2.f, whiteDiffuse)); // back wall

	tori.emplace_back(Torus(0, float3(-0.f, -0.5f, 0.f), 0.5f, 0.1f, redDiffuse));
	//cylinders.emplace_back(Cylinder(0, float3(-1.f, -0.5f, 0.f), 0.3f, 0.5f, redDiffuse));

	//spheres.emplace_back(Sphere(0, float3(-0.9f, -0.5f, 0.f), 0.3f, mirror));
	//spheres.emplace_back(Sphere(1, float3(-0.3f, -0.5f, 0.f), 0.3f, whiteDiffuse));
	//spheres.emplace_back(Sphere(0, float3(0.3f, -0.5f, 0.f), 0.3f, redDiffuse));
	//spheres.emplace_back(Sphere(3, float3(0.9f, -0.5f, 0.f), 0.3f, glass));

	//triangles.emplace_back(new Triangle(0, float3(3.0f, 0.8f, -3.0f), float3(3.0f, 0.8f, 0.0f), float3(0.0, 0.8f, 0.0f), whiteDiffuse));
	
	//LoadModel(0, "Square.obj", redDiffuse, float3(0, 1, 0), 0.1f);
	//LoadModel(0, "elephav.obj", redDiffuse, float3(0, 1, 0), 0.001f);

#ifdef WITTED_STYLE
	light = new Light(float3(0.f, 0.8f, 0.0f));
#else
	//planes.emplace_back(Plane(2, float3(0, -1, 0), 1.f, areaLight)); // ceiling
	triangles.emplace_back(Triangle(0, float3(-2.f, 0.8f, -4.0f), float3(2.f, 0.8f, -4.0f), float3(2.f, 0.8f, 0.0f), areaLight));
	triangles.emplace_back(Triangle(1, float3(-2.f, 0.8f, 0.0f), float3(-2.f, 0.8f, -4.0f), float3(2.f, 0.8f, 0.0f), areaLight));
#endif

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
	for (int i = 0; i < planes.size(); ++i)
	{
		planes[i].Intersect(ray);
	}
	for (int i = 0; i < tori.size(); ++i)
	{
		tori[i].Intersect(ray);
	}
	for (int i = 0; i < cylinders.size(); ++i)
	{
		cylinders[i].Intersect(ray);
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
	else if (type == ObjectType::TORUS)
	{
		N = tori[idx].GetNormal(I);
	}
	else if (type == ObjectType::CYLINDER)
	{
		N = cylinders[idx].GetNormal(I);
	}
	if (dot(N, D) > 0) N = -N; // hit backside / inside
	return N;
}

float3 Scene::GetShade(int idx, ObjectType type, const float3& I, const float3& N)
{
	if (IsOccluded(I, N)) return 0;
	if (type == ObjectType::PLANE) return planes[idx].GetDirectLight(light, I, N);
	if (type == ObjectType::TRIANGLE) return triangles[idx].GetDirectLight(light, I, N);
	if (type == ObjectType::SPHERE) return spheres[idx].GetDirectLight(light, I, N);
	if (type == ObjectType::TORUS) return tori[idx].GetDirectLight(light, I, N);
	if (type == ObjectType::CYLINDER) return cylinders[idx].GetDirectLight(light, I, N);
	return 0;
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
			unsigned int vertexIndex[3] = { 0 }, uvIndex[3] = { 0 }, normalIndex[3] = { 0 };
			char stupidBuffer[1024];
			fgets(stupidBuffer, 1024, file);
			int matches = sscanf(stupidBuffer, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				vertexIndex[3] = { 0 }, uvIndex[3] = { 0 }, normalIndex[3] = { 0 };
				matches = sscanf(stupidBuffer, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
				if (matches != 6) {
					vertexIndex[3] = { 0 }, uvIndex[3] = { 0 }, normalIndex[3] = { 0 };
					matches = sscanf(stupidBuffer, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
					if (matches != 3) {
						printf("File can't be read \n");
						fclose(file);
						return ;
					}
				}
			}
			/*
			triangles.emplace_back(Triangle(idx, 
				float4(temp_vertices[vertexIndex[0]].x, temp_vertices[vertexIndex[0]].y, temp_vertices[vertexIndex[0]].z, 1.f) * transform * scale,
				float4(temp_vertices[vertexIndex[1]].x, temp_vertices[vertexIndex[1]].y, temp_vertices[vertexIndex[1]].z, 1.f) * transform * scale,
				float4(temp_vertices[vertexIndex[2]].x, temp_vertices[vertexIndex[2]].y, temp_vertices[vertexIndex[2]].z, 1.f) * transform * scale,
				material));
			idx += 1;
			*/

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

const float3& Scene::GetSpecularColour(Light* light, const float3& I, const float3& N, const float3& D)
{
	float3 distToLight = normalize(light->position - I);
	float A = 4 * PI * dot(distToLight, distToLight);
	float3 B = light->GetColour() / A;
	float3 reflected = normalize(reflect(-distToLight, N));
	return pow(-dot(reflected, D), 20.0f);
}

const float3& Scene::GetAlbedo(const Ray& ray, const float3& N)
{
	float3 I = ray.O + ray.t * ray.D;
	return ray.objMaterial.colour * ray.objMaterial.Kd + ray.objMaterial.Ks * GetSpecularColour(light, I, N, ray.D);
}
