#include "precomp.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Renderer::Init()
{
	// create fp32 rgb pixel buffer to render to
	accumulator = (float4*)MALLOC64( SCRWIDTH * SCRHEIGHT * 16 );
	memset( accumulator, 0, SCRWIDTH * SCRHEIGHT * 16 );

	scene = new Scene();
}

// -----------------------------------------------------------
// Evaluate light transport
// -----------------------------------------------------------
float3 Renderer::Trace( Ray& ray, int depth )
{
	scene->FindNearest(ray);
	if (ray.objIdx == -1 || depth == 50) return 0; // or a fancy sky color
	float3 I = ray.O + ray.t * ray.D;
	float3 N = scene->GetNormal(ray.objIdx, I, ray.D);
	/* visualize normal */ // return (N + 1) * 0.5f;
	/* visualize distance */ // return 0.1f * float3( ray.t, ray.t, ray.t );

	float3 returnColour = 0;
	float3 shade = 0;

	if (ray.objMaterial.type == MaterialType::DIFFUSE)
	{
		shade = scene->GetShade(ray.objIdx, I, N);
		returnColour = scene->GetAlbedo(ray.objIdx, I, N, ray.D) * shade;
	}
	else if (ray.objMaterial.type == MaterialType::MIRROR)
	{
		float3 bias = 0.001f * N;
		bool outside = dot(ray.D, N) < 0;

		float3 reflVec = reflect(ray.D, N);
		float3 reflRayOrigin = outside ? I + bias : I - bias;
		Ray reflRay = Ray(reflRayOrigin, normalize(reflVec));

		float3 reflectionColour = Trace(reflRay, depth + 1);

		shade = float3(1);
		returnColour = reflectionColour;
	}
	else if (ray.objMaterial.type == MaterialType::GLASS)
	{
		float3 bias = 0.001f * N;
		bool outside = dot(ray.D, N) < 0;

		float3 reflVec = reflect(ray.D, N);
		float3 reflRayOrigin = outside ? I + bias : I - bias;
		Ray reflRay = Ray(reflRayOrigin, normalize(reflVec));

		float3 reflectionColour = Trace(reflRay, depth + 1);

		float dDotn = -dot(ray.D, N);
		float fresneleffect = fresnel(I, N, 1.5);

		float3 refractionColour = 0;
		if (ray.objMaterial.Ks > 0)
		{
			float ior = 1.1, eta = (outside) ? 1 / ior : ior;
			float cosi = dDotn;
			float k = 1 - eta * eta * (1 - cosi * cosi);
			float3 refrdir = ray.D * eta + N * (eta * cosi - sqrt(k));
			float3 refrRayOrigin = outside ? I - bias : I + bias;
			Ray refrRay = Ray(refrRayOrigin, normalize(refrdir));
			refractionColour = Trace(refrRay, depth + 1) * scene->GetBeersLaw(refrRay);
		}

		shade = float3(1);
		returnColour = (fresneleffect * reflectionColour + 
					   (1 - fresneleffect) * refractionColour * ray.objMaterial.Ks)
					    * ray.objMaterial.colour;
	}

	return returnColour;
}

float3 Renderer::TracePath(Ray& ray, int depth)
{
	scene->FindNearest(ray);

	if (ray.objIdx == -1 || depth == 7) return 0;
	float3 I = ray.O + ray.t * ray.D;
	float3 N = scene->GetNormal(ray.objIdx, I, ray.D);
	
	if (ray.objMaterial.type == MaterialType::DIFFUSE)
	{
		float3 bias = 0.001f * N;
		bool outside = dot(ray.D, N) < 0;
		float3 rayOrigin = outside ? I + bias : I - bias;

		float3 illumination = 0;
		for (int i = 0; i < 3; ++i)
		{
			float3 randomUnitVec = GenerateRandomVec(N);
			Ray newRay = Ray(rayOrigin, randomUnitVec);
			float3 incoming = TracePath(newRay, depth + 1);
			float3 BRDF = ray.objMaterial.colour; // this should be divided by PI
			float3 cos_i = incoming * dot(randomUnitVec, N); // irradiance
			illumination += 2.f * cos_i * BRDF; // this should be multiplied by PI but it's cancelled by the 1/PI in BRDF
		}

		return illumination / 3;
	}
	else if (ray.objMaterial.type == MaterialType::MIRROR)
	{
		float3 bias = 0.001f * N;
		bool outside = dot(ray.D, N) < 0;

		float3 reflVec = reflect(ray.D, N);
		float3 reflRayOrigin = outside ? I + bias : I - bias;
		Ray reflRay = Ray(reflRayOrigin, normalize(reflVec));

		float3 reflectionColour = TracePath(reflRay, depth + 1);

		return reflectionColour;
	}
	else if (ray.objMaterial.type == MaterialType::GLASS)
	{
		float3 bias = 0.001f * N;
		bool outside = dot(ray.D, N) < 0;

		float3 reflVec = reflect(ray.D, N);
		float3 reflRayOrigin = outside ? I + bias : I - bias;
		Ray reflRay = Ray(reflRayOrigin, normalize(reflVec));

		float3 reflectionColour = TracePath(reflRay, depth + 1);

		float dDotn = -dot(ray.D, N);
		float fresneleffect = fresnel(I, N, 1.5);

		float3 refractionColour = 0;
		if (ray.objMaterial.Ks > 0)
		{
			float ior = 1.1, eta = (outside) ? 1 / ior : ior;
			float cosi = dDotn;
			float k = 1 - eta * eta * (1 - cosi * cosi);
			float3 refrdir = ray.D * eta + N * (eta * cosi - sqrt(k));
			float3 refrRayOrigin = outside ? I - bias : I + bias;
			Ray refrRay = Ray(refrRayOrigin, normalize(refrdir));
			refractionColour = TracePath(refrRay, depth + 1);// *scene->GetBeersLaw(refrRay);
		}

		return (fresneleffect * reflectionColour +
			(1 - fresneleffect) * refractionColour * ray.objMaterial.Ks)
			* ray.objMaterial.colour;
	}
	else if (ray.objMaterial.type == MaterialType::LIGHT)
	{
		return BRIGHT;
	}

	return 0;
}

const float3& Renderer::GenerateRandomVec(const float3& N)
{
	float r1 = RandomFloat();
	float r2 = RandomFloat();
	float sinTheta = sqrtf(1 - r1 * r1);
	float phi = 2 * PI * r2;
	float x = sinTheta * cosf(phi);
	float z = sinTheta * sinf(phi);
	float3 randomVec = float3(x, r1, z);
	if (dot(randomVec, N) >= 0) return randomVec;
	return -randomVec;
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Renderer::Tick( float deltaTime )
{
	accumulatorCounter += 1;
	// animation
	static float animTime = 0;
	//scene.SetTime( animTime += deltaTime * 0.002f );
	// pixel loop
	Timer t;
	// lines are executed as OpenMP parallel tasks (disabled in DEBUG)
	#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < SCRHEIGHT; y++)
	{
		// trace a primary ray for each pixel on the line
		for (int x = 0; x < SCRWIDTH; x++)
		{
			Ray primaryRay = camera.GetPrimaryRay(x, y);
			accumulator[x + y * SCRWIDTH] += float4(TracePath(primaryRay, 0), 0);
		}
		// translate accumulator contents to rgb32 pixels
		for (int dest = y * SCRWIDTH, x = 0; x < SCRWIDTH; x++)
		{
			float4 colour = accumulator[x + y * SCRWIDTH];
			colour /= accumulatorCounter;
			screen->pixels[dest + x] = 
				RGBF32_to_RGB8( &colour );
		}
	}

	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000 / avg, rps = (SCRWIDTH * SCRHEIGHT) * fps;
	printf( "%5.2fms (%.1fps) - %.1fMrays/s\n", avg, fps, rps / 1000000 );
}

void Renderer::KeyDown(int key)
{

}

void Renderer::MouseUp(int button)
{
	if (button == 0)
		pressL = false;
	if (button == 1)
		pressR = false;
}

void Renderer::MouseDown(int button)
{
	if (button == 0)
		pressL = true;
	if (button == 1)
		pressR = true;
}

void Renderer::MouseMove(int x, int y)
{
	mousePos.x = x, mousePos.y = y;
	if (pressR)
	{
		memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
		accumulatorCounter = 0;
		camera.RotateX((y - prevY) * rotspeed);
		camera.RotateY((x - prevX) * rotspeed);
	}
	if (pressL)
	{
		memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
		accumulatorCounter = 0;
		camera.Translate(-(x - prevX) * rotspeed * 0.05, (y - prevY) * rotspeed * 0.05, 0);
		/*Ray primaryRay = camera.GetPrimaryRay(x, y);
		float4 colour = float4(Trace(primaryRay, 0), 0);*/
	}
	prevX = x;
	prevY = y;
}

void Renderer::MouseWheel(float z)
{
	camera.Translate(0, 0, z);
}