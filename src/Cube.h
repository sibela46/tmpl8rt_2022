#pragma once

namespace Tmpl8
{
	class Cube
	{
	public:
		Cube(int id, const float3& pos, const float3& size, Material m, mat4 transform = mat4::Identity(), TextureMap* t = nullptr);
		void Intersect(Ray& ray);
		float3 GetNormal(const float3& I);
		float3 GetTexture(const float3& I, const float3& N);

	public:
		float3 b[2];
		mat4 M, invM;
		int index = -1;
		Material material;
		TextureMap* texture;
	};
}