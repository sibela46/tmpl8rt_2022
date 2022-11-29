#include "precomp.h"

#define PLANE_X(o,i,m) {if((t=-(ray.O.x+o)*ray.rD.x)<ray.t)ray.t=t,ray.objIdx=i,ray.objMaterial=m;}
#define PLANE_Y(o,i,m) {if((t=-(ray.O.y+o)*ray.rD.y)<ray.t)ray.t=t,ray.objIdx=i,ray.objMaterial=m;}
#define PLANE_Z(o,i,m) {if((t=-(ray.O.z+o)*ray.rD.z)<ray.t)ray.t=t,ray.objIdx=i,ray.objMaterial=m;}

Material whiteDiffuse = { WHITE,  MaterialType::DIFFUSE, 1.0, 0.0 };
Material whiteSpecular = { WHITE,  MaterialType::DIFFUSE, 0.5, 0.5 };
Material redDiffuse = { RED,  MaterialType::DIFFUSE, 1.0, 0.0 };
Material blueDiffuse = { BLUE,  MaterialType::DIFFUSE, 1.0, 0.0 };
Material greenDiffuse = { GREEN,  MaterialType::DIFFUSE, 1.0, 0.0 };
Material mirror = { WHITE,  MaterialType::MIRROR, 0.0, 1.0 };

Scene::Scene()
{
	sceneObjects.push_back(new Plane(0, float3(1, 0, 0), 3, redDiffuse)); // left wall
	sceneObjects.push_back(new Plane(1, float3(-1, 0, 0), 2.99f, greenDiffuse)); // right wall
	sceneObjects.push_back(new Plane(2, float3(0, -1, 0), 2, blueDiffuse)); // floor
	sceneObjects.push_back(new Plane(3, float3(0, 1, 0), 1, whiteDiffuse)); // ceiling
	sceneObjects.push_back(new Plane(4, float3(0, 0, 1), 3, whiteDiffuse)); // front wall
	sceneObjects.push_back(new Plane(5, float3(0, 0, -1), 3.99f, whiteDiffuse)); // back wall
	
	sceneObjects.push_back(new Sphere(6, float3(-0.5f, 0, 0), 0.3f, blueDiffuse));
	sceneObjects.push_back(new Sphere(7, float3(0.5f, 0, 0), 0.3f, mirror));

	light = new Light(float3(0.f, 0.f, 0.f));
}

void Scene::FindNearest(Ray& ray)
{
	float t;
	if (ray.D.x < 0) PLANE_X(3, 0, redDiffuse) else PLANE_X(-2.99f, 1, greenDiffuse);
	if (ray.D.y < 0) PLANE_Y(1, 2, whiteDiffuse) else PLANE_Y(-2, 3, whiteDiffuse);
	if (ray.D.z < 0) PLANE_Z(3, 4, whiteDiffuse) else PLANE_Z(-3.99f, 5, whiteDiffuse);

	for (int i = 0; i < sceneObjects.size(); ++i)
	{
		sceneObjects[i]->Intersect(ray);
	}
}

bool Scene::IsOccluded(float3 I)
{
	return false;
}

float3 Scene::GetNormal(int idx, float3 I, float3 D)
{
	float3 N = sceneObjects[idx]->GetNormal(I);

	if (dot(N, D) > 0) N = -N; // hit backside / inside (so always use this function for GetNormal)
	return N;
}

float3 Scene::GetShade(int idx, float3 I, float3 N)
{
	return sceneObjects[idx]->GetShade(light, I, N);
}