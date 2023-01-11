#pragma once

struct Primitive {
	int index; ObjectType type; float3 centroid; float3 v1; float3 v2; float3 v3; float3 n; float size; float u; float v; Material material;
	Primitive() {}
	Primitive(int index, ObjectType type, float3 v1, float3 v2, float3 v3, float3 n, Material material) // triangle
		: index(index), type(type), v1(v1), v2(v2), v3(v3), n(n), material(material)
	{
	}
	Primitive(int index, ObjectType type, float3 v1, float size, Material material) // sphere and plane
		: index(index), type(type), v1(v1), n(v1), size(size), material(material)
	{
	}
	void CalculateCentroid()
	{
		switch (type)
		{
		case ObjectType::TRIANGLE:
		{
			centroid = (v1 + v2 + v3) * 0.3333f;
		}
		break;
		case ObjectType::SPHERE:
		{
			centroid = v1;
		}
		break;
		case ObjectType::PLANE:
		{
			centroid = n * size;
		}
		break;
		}
	}
	void Intersect(Ray& ray)
	{
		switch (type)
		{
		case ObjectType::TRIANGLE:
		{
#ifdef SSE
			IntersectTriangleSSE(ray);
#else
			IntersectTriangle(ray);
#endif
		}
		break;
		case ObjectType::SPHERE:
		{
			IntersectSphere(ray);
		}
		break;
		case ObjectType::PLANE:
		{
			IntersectPlane(ray);
		}
		break;
		}
	}
	void Intersect(const float3& O, const float3& D, const float distToLight, bool& hitObject)
	{
		switch (type)
		{
		case ObjectType::TRIANGLE:
			{
				IntersectTriangle(O, D, distToLight, hitObject);
			}
			break;
		case ObjectType::SPHERE:
			{
				IntersectSphere(O, D, distToLight, hitObject);
			}
			break;
		case ObjectType::PLANE:
			{
				IntersectPlane(O, D, distToLight, hitObject);
			}
			break;
		}
	}
	void IntersectTriangle(Ray& ray)
	{
		centroid = (v1 + v2 + v3) * 0.3333f;
		float3 edge1 = v2 - v1;
		float3 edge2 = v3 - v1;
		float3 h = cross(ray.D, edge2);
		float a = dot(edge1, h);
		if (a > -EPSILON && a < EPSILON) return; // the ray is parallel to the triangle	
		float f = 1.0 / a;
		float3 s = ray.O - v1;
		u = f * dot(s, h);
		if (u < 0.0 || u > 1.0) return;
		float3 q = cross(s, edge1);
		v = f * dot(ray.D, q);
		if (v < 0.0 || u + v > 1.0) return;
		float t = f * dot(edge2, q);
		if (t > EPSILON && t < ray.t)
		{
			ray.t = t;
			ray.objIdx = index;
			ray.objMaterial = material;
			ray.objType = ObjectType::TRIANGLE;
			ray.normal = n;
			ray.inside = dot(ray.D, n) > 0;
			return;
		}
		return;
	}
	void IntersectTriangleSSE(Ray& ray)
	{
		centroid = (v1 + v2 + v3) * 0.3333f;
		__m128 EPS4 = _mm_set_ps1(EPSILON);
		__m128 MINUSEPS4 = _mm_set_ps1(-EPSILON);
		__m128 ONE4 = _mm_set_ps1(1.0f);
		__m128 e1x4 = _mm_set_ps1(v2.x - v1.x);
		__m128 e1y4 = _mm_set_ps1(v2.y - v1.y);
		__m128 e1z4 = _mm_set_ps1(v2.z - v1.z);
		__m128 e2x4 = _mm_set_ps1(v3.x - v1.x);
		__m128 e2y4 = _mm_set_ps1(v3.y - v1.y);
		__m128 e2z4 = _mm_set_ps1(v3.z - v1.z);
		__m128 hx4 = _mm_sub_ps(_mm_mul_ps(ray.Dy4, e2z4), _mm_mul_ps(ray.Dz4, e2y4));
		__m128 hy4 = _mm_sub_ps(_mm_mul_ps(ray.Dz4, e2x4), _mm_mul_ps(ray.Dx4, e2z4));
		__m128 hz4 = _mm_sub_ps(_mm_mul_ps(ray.Dx4, e2y4), _mm_mul_ps(ray.Dy4, e2x4));
		__m128 det4 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(e1x4, hx4), _mm_mul_ps(e1y4, hy4)), _mm_mul_ps(e1z4, hz4));
		__m128 mask1 = _mm_or_ps(_mm_cmple_ps(det4, MINUSEPS4), _mm_cmpge_ps(det4, EPS4));
		__m128 inv_det4 = _mm_rcp_ps(det4);
		__m128 sx4 = _mm_sub_ps(ray.Ox4, _mm_set_ps1(v1.x));
		__m128 sy4 = _mm_sub_ps(ray.Oy4, _mm_set_ps1(v1.y));
		__m128 sz4 = _mm_sub_ps(ray.Oz4, _mm_set_ps1(v1.z));
		__m128 u4 = _mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(sx4, hx4), _mm_mul_ps(sy4, hy4)), _mm_mul_ps(sz4, hz4)), inv_det4);
		__m128 mask2 = _mm_and_ps(_mm_cmpge_ps(u4, _mm_setzero_ps()), _mm_cmple_ps(u4, ONE4));
		__m128 qx4 = _mm_sub_ps(_mm_mul_ps(sy4, e1z4), _mm_mul_ps(sz4, e1y4));
		__m128 qy4 = _mm_sub_ps(_mm_mul_ps(sz4, e1x4), _mm_mul_ps(sx4, e1z4));
		__m128 qz4 = _mm_sub_ps(_mm_mul_ps(sx4, e1y4), _mm_mul_ps(sy4, e1x4));
		__m128 v4 = _mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(ray.Dx4, qx4), _mm_mul_ps(ray.Dy4, qy4)), _mm_mul_ps(ray.Dz4, qz4)), inv_det4);
		__m128 mask3 = _mm_and_ps(_mm_cmpge_ps(v4, _mm_setzero_ps()), _mm_cmple_ps(_mm_add_ps(u4, v4), ONE4));
		__m128 newt4 = _mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(e2x4, qx4), _mm_mul_ps(e2y4, qy4)), _mm_mul_ps(e2z4, qz4)), inv_det4);
		__m128 mask4 = _mm_cmpgt_ps(newt4, _mm_setzero_ps());
		__m128 mask5 = _mm_cmplt_ps(newt4, ray.t4);
		__m128 combined = _mm_and_ps(_mm_and_ps(_mm_and_ps(_mm_and_ps(mask1, mask2), mask3), mask4), mask5);
		ray.t4 = _mm_blendv_ps(ray.t4, newt4, combined);
		ray.objIdx = index;
		ray.objMaterial = material;
		ray.objType = ObjectType::TRIANGLE;
		ray.normal = n;
	}
	void IntersectTriangle(const float3& O, const float3& D, const float distToLight, bool& hitObject)
	{
		centroid = (v1 + v2 + v3) * 0.3333f;
		float3 edge1 = v2 - v1;
		float3 edge2 = v3 - v1;
		float3 h = cross(D, edge2);
		float a = dot(edge1, h);
		if (a > -EPSILON && a < EPSILON) return; // the ray is parallel to the triangle	
		float f = 1.0 / a;
		float3 s = O - v1;
		u = f * dot(s, h);
		if (u < 0.0 || u > 1.0) return;
		float3 q = cross(s, edge1);
		v = f * dot(D, q);
		if (v < 0.0 || u + v > 1.0) return;
		float t = f * dot(edge2, q);
		if (t > EPSILON && t < distToLight)
		{
			hitObject = true;
			return;
		}
		return;
	}
	void IntersectPlane(Ray& ray)
	{
		float t = -(dot(ray.O, n) + size) / (dot(ray.D, n));
		if (t < ray.t && t > 0)
		{
			ray.t = t;
			ray.objIdx = index;
			ray.objMaterial = material;
			ray.objType = ObjectType::PLANE;
			ray.normal = n;
			ray.inside = dot(ray.D, n) > 0;
		}
	}
	void IntersectPlane(float3 O, float3 D, const float distToLight, bool& hitObject)
	{
		float t = -(dot(O, n) + size) / (dot(D, n));
		if (t < distToLight && t > 0)
		{
			hitObject = true;
		}
	}
	void IntersectSphere(Ray& ray)
	{
		float3 oc = ray.O - v1;
		float b = dot(oc, ray.D);
		float c = dot(oc, oc) - size * size;
		float t, d = b * b - c;
		if (d <= 0) return;
		d = sqrtf(d), t = -b - d;
		if (t < ray.t && t > 0)
		{
			float3 I = ray.O + t * ray.D;
			n = normalize(I - v1);
			u = 0.5 - atan2(n.z, n.x) * (1 / PI);
			v = 0.5 - asin(n.y) * (1 / PI);
			ray.t = t, ray.objIdx = index, ray.objMaterial = material, ray.objType = ObjectType::SPHERE, ray.normal = n, ray.inside = dot(ray.D, n) > 0;
			return;
		}
		t = d - b;
		if (t < ray.t && t > 0)
		{
			float3 I = ray.O + t * ray.D;
			n = normalize(I - v1);
			u = 0.5 - atan2(n.z, n.x) * (1 / PI);
			v = 0.5 - asin(n.y) * (1 / PI);
			ray.t = t, ray.objIdx = index, ray.objMaterial = material, ray.objType = ObjectType::SPHERE, ray.normal = n, ray.inside = dot(ray.D, n) > 0;
			return;
		}
	}
	void IntersectSphere(float3 O, float3 D, const float distToLight, bool& hitObject)
	{
		float3 oc = O - v1;
		float b = dot(oc, D);
		float c = dot(oc, oc) - size * size;
		float t, d = b * b - c;
		if (d <= 0) return;
		d = sqrtf(d), t = -b - d;
		if (t > 0 && t < distToLight)
		{
			hitObject = true;
			return;
		}
		t = d - b;
		if (t > 0 && t < distToLight)
		{
			hitObject = true;
			return;
		}
	}
};