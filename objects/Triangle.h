#pragma once

namespace Tmpl8
{
	class Triangle
	{
	public:
		Triangle(int idx, const float4& vertex1, const float4& vertex2, const float4& vertex3, const float3& normal, Material material, TextureMap* texture=nullptr);
		void Intersect(Ray& ray);
		float3 GetNormal();
		float3 GetTexture(const float3& I, const float3& N);

	public:
		float3 v0, v1, v2, normal, centroid;
		float u, v;
		int index;
		Material material;
		TextureMap* texture;
	};
}