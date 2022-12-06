#include "precomp.h"

/**
As reference we use the github repo from:
https://github.com/lishicao/RayTracer/blob/master/myRayTracer/GeometricObjects/Primitives/Torus.cpp
**/


Torus::Torus(int id, const float3& p, float r, float d,  Material m)
{
	index = id;
	radius = r;
	radius2 = d;
	position = p;
	material = m;
}

float3 Torus::GetNormal(const float3& I)
{
	float param_squared = radius * radius + radius2 * radius2;

	float x = I.x;
	float y = I.y;
	float z = I.z;
	float sum_squared = x * x + y * y + z * z;

	float3 normal;
	normal.x = 4.0 * x * (sum_squared - param_squared);
	normal.y = 4.0 * y * (sum_squared - param_squared + 2.0 * radius * radius);
	normal.z = 4.0 * z * (sum_squared - param_squared);

	return normalize(normal);;
}

void Torus::Intersect(Ray& ray)
{
	float x1 = ray.O.x; float y1 = ray.O.y; float z1 = ray.O.z;
	float d1 = ray.D.x; float d2 = ray.D.y; float d3 = ray.D.z;

	float roots[4];	// solution array for the quartic equation

	// define the coefficients of the quartic equation

	float sum_d_sqrd = d1* d1 + d2 * d2 + d3 * d3;
	float e = x1 * x1 + y1 * y1 + z1 * z1 - radius * radius - radius2 * radius2;
	float f = x1 * d1 + y1 * d2 + z1 * d3;
	float four_a_sqrd = 4.0 * radius * radius;

	float coeffs[5];	// coefficient array for the quartic equation
	coeffs[0] = e * e - four_a_sqrd * (radius2 * radius2 - y1 * y1); 	// constant term
	coeffs[1] = 4.0 * f * e + 2.0 * four_a_sqrd * y1 * d2;
	coeffs[2] = 2.0 * sum_d_sqrd * e + 4.0 * f * f + four_a_sqrd * d2 * d2;
	coeffs[3] = 4.0 * sum_d_sqrd * f;
	coeffs[4] = sum_d_sqrd * sum_d_sqrd;  					// coefficient of t^4

	// find roots of the quartic equation

	int num_real_roots = SolveQuartic(coeffs, roots);

	if (num_real_roots == 0)  // ray misses the torus
		return;

	// the roots array is not sorted
	for (int j = 0; j < num_real_roots; j++) {
		if (roots[j] > EPSILON && roots[j] < ray.t) {
			ray.t = roots[j];
		}
	}

	ray.objIdx = index, ray.objMaterial = material, ray.objType = ObjectType::TORUS;
	return;

}

float3 Torus::GetDirectLight(Light* light, const float3& I, const float3& N)
{
	float3 dirToLight = (light->GetPosition() - I);
	float dotProduct = max(0.f, dot(normalize(dirToLight), N));
	return light->GetEmission() * light->GetColour() * dotProduct * (1 / PI);
}

float3 Torus::GetSpecularColour(Light* light, const float3& I, const float3& N, const float3& D)
{
	float3 distToLight = normalize(light->position - I);
	float A = 4 * PI * dot(distToLight, distToLight);
	float3 B = light->GetColour() / A;
	float3 reflected = normalize(reflect(-distToLight, N));
	return pow(-dot(reflected, D), 20.0f);
}

float3 Torus::GetAlbedo(Light* light, const float3& I, const float3& N, const float3& D)
{
	return material.colour * material.Kd + material.Ks * GetSpecularColour(light, I, N, D);
}


#ifndef M_PI
#define M_PI PI
#endif

#define     EQN_EPS     EPSILON

#define	IsZero(x)	((x) > -EQN_EPS && (x) < EQN_EPS)

#ifndef CBRT
#define     cbrt(x)  ((x) > 0.0 ? pow((float)(x), 1.0/3.0) : \
			  		 ((x) < 0.0 ? -pow((float)-(x), 1.0/3.0) : 0.0))
#endif


int Torus::SolveQuadric(float c[3], float s[2]) {
	float p, q, D;


	p = c[1] / (2 * c[2]);
	q = c[0] / c[2];

	D = p * p - q;

	if (IsZero(D)) {
		s[0] = -p;
		return 1;
	}
	else if (D > 0) {
		float sqrt_D = sqrt(D);

		s[0] = sqrt_D - p;
		s[1] = -sqrt_D - p;
		return 2;
	}
	else
		return 0;
}


int Torus::SolveCubic(float c[4], float s[3]) {
	int     i, num;
	float  sub;
	float  A, B, C;
	float  sq_A, p, q;
	float  cb_p, D;

	A = c[2] / c[3];
	B = c[1] / c[3];
	C = c[0] / c[3];


	sq_A = A * A;
	p = 1.0 / 3 * (-1.0 / 3 * sq_A + B);
	q = 1.0 / 2 * (2.0 / 27 * A * sq_A - 1.0 / 3 * A * B + C);


	cb_p = p * p * p;
	D = q * q + cb_p;

	if (IsZero(D)) {
		if (IsZero(q)) {
			s[0] = 0;
			num = 1;
		}
		else {
			float u = cbrt(-q);
			s[0] = 2 * u;
			s[1] = -u;
			num = 2;
		}
	}
	else if (D < 0) {
		float phi = 1.0 / 3 * acos(-q / sqrt(-cb_p));
		float t = 2 * sqrt(-p);

		s[0] = t * cos(phi);
		s[1] = -t * cos(phi + M_PI / 3);
		s[2] = -t * cos(phi - M_PI / 3);
		num = 3;
	}
	else {
		float sqrt_D = sqrt(D);
		float u = cbrt(sqrt_D - q);
		float v = -cbrt(sqrt_D + q);

		s[0] = u + v;
		num = 1;
	}

	sub = 1.0 / 3 * A;

	for (i = 0; i < num; ++i)
		s[i] -= sub;

	return num;
}



int Torus::SolveQuartic(float c[5], float s[4]) {
	float  coeffs[4];
	float  z, u, v, sub;
	float  A, B, C, D;
	float  sq_A, p, q, r;
	int     i, num;


	A = c[3] / c[4];
	B = c[2] / c[4];
	C = c[1] / c[4];
	D = c[0] / c[4];


	sq_A = A * A;
	p = -3.0 / 8 * sq_A + B;
	q = 1.0 / 8 * sq_A * A - 1.0 / 2 * A * B + C;
	r = -3.0 / 256 * sq_A * sq_A + 1.0 / 16 * sq_A * B - 1.0 / 4 * A * C + D;

	if (IsZero(r)) {

		coeffs[0] = q;
		coeffs[1] = p;
		coeffs[2] = 0;
		coeffs[3] = 1;

		num = SolveCubic(coeffs, s);

		s[num++] = 0;
	}
	else {

		coeffs[0] = 1.0 / 2 * r * p - 1.0 / 8 * q * q;
		coeffs[1] = -r;
		coeffs[2] = -1.0 / 2 * p;
		coeffs[3] = 1;

		(void)SolveCubic(coeffs, s);



		z = s[0];



		u = z * z - r;
		v = 2 * z - p;

		if (IsZero(u))
			u = 0;
		else if (u > 0)
			u = sqrt(u);
		else
			return 0;

		if (IsZero(v))
			v = 0;
		else if (v > 0)
			v = sqrt(v);
		else
			return 0;

		coeffs[0] = z - u;
		coeffs[1] = q < 0 ? -v : v;
		coeffs[2] = 1;

		num = SolveQuadric(coeffs, s);

		coeffs[0] = z + u;
		coeffs[1] = q < 0 ? v : -v;
		coeffs[2] = 1;

		num += SolveQuadric(coeffs, s + num);
	}


	sub = 1.0 / 4 * A;

	for (i = 0; i < num; ++i)
		s[i] -= sub;

	return num;
}