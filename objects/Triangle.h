#pragma once

namespace Tmpl8
{
	class Triangle
	{
	public:
		Triangle(int idx, const float4& vertex1, const float4& vertex2, const float4& vertex3, Material material);
		void Intersect(Ray& ray);
		float3 GetNormal(const float3& I);
		virtual float3	GetDirectLight(Light* light, const float3& I, const float3& N);
		virtual float3	GetSpecularColour(Light* light, const float3& I, const float3& N, const float3& D);

	public:
		float3 v0, v1, v2;
		int index;
		Material material;
	};
}