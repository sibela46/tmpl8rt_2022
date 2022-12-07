#pragma once

namespace Tmpl8
{
	class Triangle
	{
	public:
		Triangle(int idx, const float4& vertex1, const float4& vertex2, const float4& vertex3, Material material);
		void Intersect(Ray& ray);
		float3 GetNormal(const float3& I);
		float3 GetTexture(const float3& I, const float3& N);

	public:
		float3 v0, v1, v2;
		float u, v;
		int index;
		Material material;
	};
}