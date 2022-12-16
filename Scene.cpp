#include "precomp.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "lib/tiny_obj_loader.h"

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
	planes.emplace_back(Plane(0, float3(1, 0, 0), 2.f, purpleDiffuse)); // left wall
	planes.emplace_back(Plane(1, float3(-1, 0, 0), 2.f, blueDiffuse)); // right wall
	planes.emplace_back(Plane(2, float3(0, -1, 0), 2.f, whiteDiffuse)); // ceiling
	planes.emplace_back(Plane(3, float3(0, 1, 0), 1.f, whiteDiffuse)); // floor
	planes.emplace_back(Plane(4, float3(0, 0, 1), 3.f, whiteDiffuse)); // front wall
	planes.emplace_back(Plane(5, float3(0, 0, -1), 2.f, whiteDiffuse)); // back wall
	
	//spheres.emplace_back(Sphere(0, float3(-1.3f, 0.f, 0.f), 0.6f, whiteDiffuse, new TextureMap("\\assets\\universe.jpg")));
	//spheres.emplace_back(Sphere(1, float3(-0.2f, 0.f, 0.f), 0.5f, mirror));
	//spheres.emplace_back(Sphere(2, float3(0.7f, 0.f, 0.f), 0.4f, glass));
	//spheres.emplace_back(Sphere(3, float3(1.4f, 0.f, 0.f), 0.3f, purpleDiffuse));

	//cylinders.emplace_back(Cylinder(0, float3(0.0, -2.f, 3.f), 0.3f, 0.3f, redDiffuse));
	
	//cubes.emplace_back(Cube(0, float3(0), float3(0.5f), redDiffuse, mat4::Translate(0.5f, -0.75f, 0.f)*mat4::RotateY(10)));

	//triangles.emplace_back(Triangle(0, float3(-1.0f, 0.0f, 0.0f), float3(1.0f, 1.0f, 0.0f), float3(1.0f, 0.0f, 1.0f), whiteDiffuse));

	//skydomeTexture = new TextureMap("\\assets\\sky.jfif");

#ifdef WHITTED_STYLE
	light = new Light(float3(0.f, 0.5f, 0.0f));
#else
	triangles.emplace_back(Triangle(0, float3(-1.f, 1.8f, -1.f), float3(1.f, 1.8f, -1.f), float3(1.f, 1.8f, 1.f), float3(0, -1, 0), areaLight));
	triangles.emplace_back(Triangle(1, float3(-1.f, 1.8f, 1.f), float3(-1.f, 1.8f, -1.f), float3(1.f, 1.8f, 1.f), float3(0, -1, 0), areaLight));
#endif

	//LoadModelNew(0, "assets\\ChristmasTree.obj", greenDiffuse, float3(10.0f, -15.f, 10.0f), 0.01f);
	LoadModelNew(2, "assets\\bunny.obj", whiteDiffuse, float3(2.0f, -2.f, 0.0f), 0.5f);
}

void Scene::FindNearest(Ray& ray, bool isShadowRay)
{
	float t;

	for (int i = 0; i < triangles.size(); ++i)
	{
		triangles[i].Intersect(ray);
	}
	for (int i = 0; i < models.size(); ++i)
	{
		models[i].IntersectBVH(ray, Bvh::rootNodeIdx);
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
	for (int i = 0; i < cylinders.size(); ++i)
	{
		cylinders[i].Intersect(ray);
	}
	for (int i = 0; i < planes.size(); ++i)
	{
		planes[i].Intersect(ray);
	}
}

bool Scene::IsOccluded(const Ray& ray)
{
	float3 bias = 0.001f * ray.normal;
	float3 I = ray.O + ray.t * ray.D;
	float3 origin = ray.inside ? I - bias : I + bias;
	float3 dirToLight = (light->position - I);
	Ray rayToLight = Ray(origin, normalize(dirToLight));
	FindNearest(rayToLight, true);

	return rayToLight.t*rayToLight.t < dot(dirToLight, dirToLight);
}

float3 Scene::GetNormal(int idx, ObjectType type, const float3& I, const float3& D)
{
	float3 N;
	if (type == ObjectType::PLANE)
	{
		N = planes[idx].GetNormal();
	}
	else if (type == ObjectType::TRIANGLE)
	{
		N = triangles[idx].GetNormal();
	}
	else if (type == ObjectType::SPHERE)
	{
		N = spheres[idx].GetNormal(I);
	}
	else if (type == ObjectType::CUBE)
	{
		N = cubes[idx].GetNormal(I);
	}
	else if (type == ObjectType::CYLINDER)
	{
		N = cylinders[idx].GetNormal(I);
	}
	else if (type == ObjectType::TORUS)
	{
		N = tori[idx].GetNormal(I);
	}
	if (dot(N, D) > 0) N = -N; // hit backside / inside
	return N;
}

float3 Scene::GetNormal(const Ray& ray)
{
	if (ray.inside) return -ray.normal; 
	return ray.normal; // hit backside / inside
}

float3 Scene::GetShade(const Ray& ray)
{
	if (IsOccluded(ray)) return 0;
	float3 I = ray.O + ray.t * ray.D;
	float3 dirToLight = (light->GetPosition() - I);
	float dotProduct = max(0.f, dot(normalize(dirToLight), ray.normal));
	float attenuation = 1 / length(dirToLight);
	float invPi = 1 / PI;
	return light->GetEmission() * light->GetColour() * dotProduct * attenuation * invPi;
}

float3 Scene::GetBeersLaw(Ray& ray)
{
	if (ray.objIdx == -1) return 1;
	float distanceTravelled = ray.t;
	float3 absorbance = (ray.objMaterial.colour) * 0.15f * (-distanceTravelled);
	float3 N = GetNormal(ray);
	bool outside = dot(ray.D, N) > 0;
	if (outside)
	{
		return float3(pow(E, absorbance.x), pow(E, absorbance.y), pow(E, absorbance.z));
	}
	return 1;
}

void Scene::LoadModel(int idx, const char* fileName, Material material, const float3& offset, float scale)
{
	vector<Triangle> Model;

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
		Model.emplace_back(Triangle(idx, out_vertices[i], out_vertices[i + 1], out_vertices[i + 2], out_normals[i], material));
		idx += 1;
	}
}

void Scene::LoadModelNew(int triIdx, const char* fileName, Material material, const float3& offset, float scale)
{
	std::vector<Triangle> mesh;
	mat4 transform = mat4::Translate(offset.x, offset.y, offset.z) ;

	std::string inputfile = fileName;
	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = "./"; // Path to material files

	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(inputfile, reader_config)) {
		if (!reader.Error().empty()) {
			std::cerr << "TinyObjReader: " << reader.Error();
		}
		exit(1);
	}

	if (!reader.Warning().empty()) {
		std::cout << "TinyObjReader: " << reader.Warning();
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
			std::vector<float4> vertices;
			float3 normal = float3(0);

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
				float3 newVertex = float4(vx, vy, vz, 1.f) * scale * transform;
				vertices.push_back(newVertex);

				// Check if `normal_index` is zero or positive. negative = no normal data
				if (idx.normal_index >= 0) {
					tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
					tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
					tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
					normal = float3(nx, ny, nz);
				}

				// Check if `texcoord_index` is zero or positive. negative = no texcoord data
				if (idx.texcoord_index >= 0) {
					tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
					tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
				}

				// Optional: vertex colors
				// tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
				// tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
				// tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
			}
			Triangle newTri = Triangle(triIdx, vertices[0], vertices[1], vertices[2], normal, material);
			mesh.push_back(newTri);

			index_offset += fv;
			triIdx += 1;
			// per-face material
			shapes[s].mesh.material_ids[f];
		}
	}

	Bvh b(mesh);
	b.BuildBVH();
	models.emplace_back(b);
}

float3 Scene::GetAlbedo(Ray& ray, const float3& N)
{
	return ray.objMaterial.colour;
	/*if (ray.objType == ObjectType::SPHERE) return spheres[ray.objIdx].GetTexture(ray.IntersectionPoint(), N);
	if (ray.objType == ObjectType::PLANE) return planes[ray.objIdx].GetTexture(ray.IntersectionPoint(), N);
	if (ray.objType == ObjectType::CUBE) return cubes[ray.objIdx].GetTexture(ray.IntersectionPoint(), N);
	if (ray.objType == ObjectType::TRIANGLE) return triangles[ray.objIdx].GetTexture(ray.IntersectionPoint(), N);
	if (ray.objType == ObjectType::CYLINDER) return cylinders[ray.objIdx].GetTexture(ray.IntersectionPoint(), N);
	return (0, 1, 0);*/
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