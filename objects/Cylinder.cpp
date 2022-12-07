#include "precomp.h"

Cylinder::Cylinder(int id, const float3& p, float r, float d,  Material m)
{
	index = id;
	radius = r;
	height = d;
	position = p;
	material = m;
}

void Cylinder::Intersect(Ray& ray)
{
	float a = (ray.D.x * ray.D.x) + (ray.D.z * ray.D.z);
	float b = 2 * (ray.D.x * (ray.O.x - position.x) + ray.D.z * (ray.O.z - position.z));
	float c = (ray.O.x - position.x) * (ray.O.x - position.x) + (ray.O.z - position.z) * (ray.O.z - position.z) - (radius * radius);

	float delta = b * b - 4 * (a * c);
	if (fabs(delta) < 0.001) return;
	if (delta < 0.0) return;

	float t1 = (-b - sqrt(delta)) / (2 * a);
	float t2 = (-b + sqrt(delta)) / (2 * a);
	float t;

	if (t1 > t2) t = t2;
	else t = t1;

	float r = ray.O.y + t * ray.D.y;

	if ((r >= position.y) && (r <= position.y + height)) {
		ray.t = t, ray.objIdx = index, ray.objMaterial = material, ray.objType = ObjectType::CYLINDER;
		return;
	}
	else return;
}

float3 Cylinder::GetNormal(const float3& I)
{
	float3 n = float3(I.x - position.x, 0, I.z - position.z);
	return normalize(n);
}

float3 Cylinder::GetTexture(const float3& I, const float3& N)
{
	return material.colour;
}