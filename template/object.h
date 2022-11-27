#pragma once

#include <direct.h>

namespace Tmpl8 {

class Object
{
public:
	Object() = default;
	Object(int idx, float3 vertex0, float3 vertex1, float3 vertex2, float radius, float distance, Material mat, ObjectType type)
		: idx(idx), v0(vertex0), v1(vertex1), v2(vertex2), radius(radius), dist(distance), mat(mat), type(type)
	{
	}
	float3 GetNormal(const float3 I) const
	{
		if (type == ObjectType::TRIANGLE)
		{
			float3 edge1 = v1 - v0;
			float3 edge2 = v2 - v0;

			return normalize(cross(edge1, edge2));
		}
		else if (type == ObjectType::PLANE)
		{
			return normalize(v0);
		}
		else if (type == ObjectType::SPHERE)
		{
			return normalize(I - v0) * (1 / radius);
		}
	}
	float3 GetDirectLight(Ray ray, Light light) const
	{
		if (mat.type == MaterialType::DIFFUSE) /* diffuse */
		{
			float3 distToLight = (light.position - ray.IntersectionPoint());
			float3 normal = GetNormal(ray.IntersectionPoint());
			float dotProduct = max(0.f, dot(normalize(distToLight), normal));
			return light.emission * light.colour * dotProduct * (1/PI);
		}
		else if (mat.type == MaterialType::MIRROR) /* mirror */
		{
			return 0;
		}
		else if (mat.type == MaterialType::GLASS) /* glass */
		{

		}
	}
	float3 GetSpecularLight(Ray ray, Light light) const
	{
		float3 distToLight = normalize(light.position - ray.IntersectionPoint());
		float A = 4 * PI * dot(distToLight, distToLight);
		float3 B = light.colour / A;
		float3 reflected = normalize(reflect(-distToLight, ray.normal));
		return pow(-dot(reflected, ray.D), 20.0f);
	}
	float3 GetIndirectLight(Ray ray, Light light) const
	{
		return 0; //0.1f * float3(1, 1, 1); // removing this results in weird white line?
	}
	void Intersect(Ray& ray, Light light) const
	{
		if (type == ObjectType::TRIANGLE)
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
				ray.normal = GetNormal(ray.IntersectionPoint());
				ray.mat = mat;
				ray.mat.colour *= (GetDirectLight(ray, light) + GetIndirectLight(ray, light));
				return;
			}
			return;
		}
		else if (type == ObjectType::SPHERE)
		{
			float3 oc = ray.O - v0;
			float b = dot(oc, ray.D);
			float c = dot(oc, oc) - radius*radius;
			float t, d = b * b - c;
			if (d <= 0) return;
			d = sqrtf(d), t = -b - d;
			if (t < ray.t && t > 0)
			{
				ray.t = t, ray.objIdx = idx, ray.normal = GetNormal(ray.IntersectionPoint());
				ray.mat = mat;
				ray.mat.colour *= (ray.mat.Kd * ray.mat.colour * GetDirectLight(ray, light) + ray.mat.Ks * GetSpecularLight(ray, light) + GetIndirectLight(ray, light));
				return;
			}
			t = d - b;
			if (t < ray.t && t > 0)
			{
				ray.t = t, ray.objIdx = idx, ray.normal = GetNormal(ray.IntersectionPoint());
				ray.mat = mat;
				ray.mat.colour = (ray.mat.Kd * ray.mat.colour * GetDirectLight(ray, light) + ray.mat.Ks * GetSpecularLight(ray, light) + GetIndirectLight(ray, light));
				return;
			}
		}
		else if (type == ObjectType::PLANE)
		{
			float3 normal = GetNormal(ray.IntersectionPoint());
			float t = -(dot(ray.O, normal) + dist) / (dot(ray.D, normal));
			if (t < ray.t && t > 0) ray.t = t, ray.objIdx = idx, ray.normal = GetNormal(ray.IntersectionPoint()), ray.mat.colour = mat.colour * (GetDirectLight(ray, light) + GetIndirectLight(ray, light)), ray.mat.type = mat.type;
		}
	}
	int idx;
	float3 v0, v1, v2;
	float radius, dist;
	Material mat;
	ObjectType type;
};

}