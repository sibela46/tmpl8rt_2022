#pragma once

namespace Tmpl8
{
	class Triangle : public Object
	{
	public:
		Triangle(int idx, const float3& vertex1, const float3& vertex2, const float3& vertex3, Material material);
		void Intersect(Ray& ray) override;
		float3 GetNormal(const float3& I) override;

	public:
		float3 v0, v1, v2;
	};
}