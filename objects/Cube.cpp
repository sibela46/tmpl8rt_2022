#include "precomp.h"

Cube::Cube(int id, const float3& pos, const float3& size, Material m, mat4 transform, TextureMap* t)
{
	index = id;
	b[0] = pos - 0.5f * size, b[1] = pos + 0.5f * size;
	material = m;
	M = transform, invM = transform.FastInvertedTransformNoScale();
	texture = t;
}

void Cube::Intersect(Ray& ray)
{
	// 'rotate' the cube by transforming the ray into object space
	// using the inverse of the cube transform.
	float3 O = TransformPosition(ray.O, invM);
	float3 D = TransformVector(ray.D, invM);
	float rDx = 1 / D.x, rDy = 1 / D.y, rDz = 1 / D.z;
	int signx = D.x < 0, signy = D.y < 0, signz = D.z < 0;
	float tmin = (b[signx].x - O.x) * rDx;
	float tmax = (b[1 - signx].x - O.x) * rDx;
	float tymin = (b[signy].y - O.y) * rDy;
	float tymax = (b[1 - signy].y - O.y) * rDy;
	if (tmin > tymax || tymin > tmax) return;
	tmin = max(tmin, tymin), tmax = min(tmax, tymax);
	float tzmin = (b[signz].z - O.z) * rDz;
	float tzmax = (b[1 - signz].z - O.z) * rDz;
	if (tmin > tzmax || tzmin > tmax) return;
	tmin = max(tmin, tzmin), tmax = min(tmax, tzmax);
	if (tmin > 0)
	{
		if (tmin < ray.t) ray.t = tmin, ray.objIdx = index, ray.objMaterial = material, ray.objType = ObjectType::CUBE;
	}
	else if (tmax > 0)
	{
		if (tmax < ray.t) ray.t = tmax, ray.objIdx = index; ray.objMaterial = material, ray.objType = ObjectType::CUBE;
	}
}

float3 Cube::GetNormal(const float3& I)
{
	// transform intersection point to object space
	float3 objI = TransformPosition(I, invM);
	// determine normal in object space
	float3 N = float3(-1, 0, 0);
	float d0 = fabs(objI.x - b[0].x), d1 = fabs(objI.x - b[1].x);
	float d2 = fabs(objI.y - b[0].y), d3 = fabs(objI.y - b[1].y);
	float d4 = fabs(objI.z - b[0].z), d5 = fabs(objI.z - b[1].z);
	float minDist = d0;
	if (d1 < minDist) minDist = d1, N.x = 1;
	if (d2 < minDist) minDist = d2, N = float3(0, -1, 0);
	if (d3 < minDist) minDist = d3, N = float3(0, 1, 0);
	if (d4 < minDist) minDist = d4, N = float3(0, 0, -1);
	if (d5 < minDist) minDist = d5, N = float3(0, 0, 1);
	// return normal in world space
	return TransformVector(N, M);
}

float3 Cube::GetTexture(const float3& I, const float3& N)
{
	if (texture == nullptr) return material.colour;

	float3 a = cross(N, float3(1, 0, 0));
	float3 b = cross(N, float3(0, 1, 0));

	float3 max_ab = dot(a, a) < dot(b, b) ? b : a;

	float3 c = cross(N, float3(0, 0, 1));

	float3 u = normalize(dot(max_ab, max_ab) < dot(c, c) ? c : max_ab);
	float3 v = cross(N, u);
	return texture->GetColourAt(fmod(abs(dot(u, I)), 1.f), fmod(abs(dot(v, I)), 1.f));
}
