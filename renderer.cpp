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
	if (ray.objIdx == -1 || depth == 30) return 0; // or a fancy sky color
	float3 I = ray.O + ray.t * ray.D;
	float3 N = scene->GetNormal(ray.objIdx, I, ray.D);
	/* visualize normal */ // return (N + 1) * 0.5f;
	/* visualize distance */ // return 0.1f * float3( ray.t, ray.t, ray.t );

	float3 returnColour = 0;

	if (ray.objMaterial.type == MaterialType::DIFFUSE)
	{
		returnColour += scene->GetAlbedo(ray.objIdx, I, N, ray.D) * scene->GetShade(ray.objIdx, I, N);
	}
	else if (ray.objMaterial.type == MaterialType::MIRROR)
	{
		float3 bias = 0.001f * N;
		bool outside = dot(ray.D, N) < 0;

		float3 reflVec = reflect(ray.D, N);
		float3 reflRayOrigin = outside ? I + bias : I - bias;
		Ray reflRay = Ray(reflRayOrigin, reflVec);

		returnColour += Trace(reflRay, depth + 1);
	}
	else if (ray.objMaterial.type == MaterialType::GLASS)
	{
		bool outside = dot(ray.D, N) < 0;
		float3 bias = 0.001f * N;
		float3 origin = outside ? I + bias : I - bias;

		float3 reflVec = normalize(reflect(ray.D, N));
		Ray reflRay = Ray(origin, reflVec);
		float3 reflectionColour = Trace(reflRay, depth + 1);

		float kr = fresnel(ray.D, N, 1.5f);
		float3 refractionColour;
		if (kr < 1) {
			float3 refractionDirection = normalize(refract(ray.D, N, 1.5));
			float3 refractionRayOrig = outside ? I - bias : I + bias;
			Ray refrRay = Ray(refractionRayOrig, refractionDirection);
			refractionColour = Trace(refrRay, depth + 1);
		}
		

		returnColour += kr * reflectionColour + (1 - kr) * refractionColour;
	}

	return returnColour;
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Renderer::Tick( float deltaTime )
{
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
			accumulator[x + y * SCRWIDTH] = float4(Trace(primaryRay, 0), 0);// +float4(Trace(primaryRay, 0), 0) + float4(Trace(primaryRay, 0), 0) + float4(Trace(primaryRay, 0), 0)) / 4;
		}
		// translate accumulator contents to rgb32 pixels
		for (int dest = y * SCRWIDTH, x = 0; x < SCRWIDTH; x++)
			screen->pixels[dest + x] = 
				RGBF32_to_RGB8( &accumulator[x + y * SCRWIDTH] );
	}
	
	KeyPress();

	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000 / avg, rps = (SCRWIDTH * SCRHEIGHT) * fps;
	printf( "%5.2fms (%.1fps) - %.1fMrays/s\n", avg, fps, rps / 1000000 );
}

// -----------------------------------------------------------
// Main loop for handling input
// -----------------------------------------------------------
void Renderer::KeyPress()
{
}