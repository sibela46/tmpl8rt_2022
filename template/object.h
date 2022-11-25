#pragma once

#include <direct.h>

namespace Tmpl8 {

class Object
{
public:
	Object() = default;
	Object(int idx, float3 vertex0, float3 vertex1, float3 vertex2, float radius, float distance, Material mat, string type)
		: idx(idx), v0(vertex0), v1(vertex1), v2(vertex2), radius(radius), dist(distance), mat(mat), type(type)
	{
	}
	float3 GetNormal() const
	{
		if (type == "triangle")
		{
			float3 edge1 = v1 - v0;
			float3 edge2 = v2 - v0;

			return normalize(cross(edge1, edge2));
		}
		else if (type == "plane")
		{
			return v0;
		}
	}
	float3 GetDirectLight(Ray ray, Light light) const
	{
		if (mat.type == 0) /* diffuse */
		{
			float3 distToLight = (light.position - ray.IntersectionPoint());
			//printf("%d %d %d\n", distToLight.x, distToLight.y, distToLight.z);
			float3 D = dot(normalize(distToLight), GetNormal()) * mat.colour * (1/length(distToLight)*length(distToLight));
			return D;
		}
		else if (mat.type == 1) /* mirror */
		{

		}
		else if (mat.type == 2) /* glass */
		{

		}
	}
	void Intersect(Ray& ray, Light light) const
	{
		if (type == "triangle")
		{
			float3 edge1 = v1 - v0;
			float3 edge2 = v2 - v0;
			float3 h = cross(ray.D, edge2);
			float a = dot(edge1, h);
			if (a > -EPSILON && a < EPSILON) return; // the ray is parallel to the triangle
			float f = 1.0 / a;
			float3 s = ray.O - v0;
			float u = f * dot(s, h);
			if (u < 0.0 || u > 1.0) return;
			float3 q = cross(s, edge1);
			float v = f * dot(ray.D, q);
			if (v < 0.0 || u + v > 1.0) return;
			float t = f * dot(edge2, q);
			if (t > EPSILON)
			{
				ray.t = t;
				ray.objIdx = idx;
				ray.mat = mat;
				return;
			}
			return;
		}
		else if (type == "sphere")
		{
			float3 oc = ray.O - v0;
			float b = dot(oc, ray.D);
			float c = dot(oc, oc) - radius*radius;
			float t, d = b * b - c;
			if (d <= 0) return;
			d = sqrtf(d), t = -b - d;
			if (t < ray.t && t > 0)
			{
				ray.t = t, ray.objIdx = idx, ray.mat = mat;
				return;
			}
			t = d - b;
			if (t < ray.t && t > 0)
			{
				ray.t = t, ray.objIdx = idx, ray.mat = mat;
				return;
			}
		}
		else if (type == "plane")
		{
			float3 normal = GetNormal();
			float t = -(dot(ray.O, normal) + dist) / (dot(ray.D, normal));
			if (t < ray.t && t > 0) ray.t = t, ray.objIdx = idx, ray.mat.colour = GetDirectLight(ray, light), ray.mat.type = mat.type;
		}
	}
	int idx;
	float3 v0, v1, v2;
	float radius, dist;
	Material mat;
	string type;
};

}