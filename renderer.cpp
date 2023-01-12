#include "precomp.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Renderer::Init()
{
	// create fp32 rgb pixel buffer to render to
	accumulator = (float4*)MALLOC64( SCRWIDTH * SCRHEIGHT * 16 );
	memset( accumulator, 0, SCRWIDTH * SCRHEIGHT * 16 );

	data = new DataCollector();
	scene = new Scene(data);
}

// -----------------------------------------------------------
// Evaluate light transport
// -----------------------------------------------------------
float3 Renderer::Trace( Ray& ray, int depth )
{
	scene->FindNearest(ray);
	if (ray.objIdx == -1 || depth >= MAX_DEPTH) return scene->GetSkydomeTexture(ray); // or a fancy sky color
	
	if (depth == 0) data->UpdateIntersectedPrimitives();

	float3 I = ray.O + ray.t * ray.D;
	float3 N = scene->GetNormal(ray);

	if (ray.objMaterial.type == MaterialType::DIFFUSE)
	{
#ifdef WHITTED_STYLE
		return scene->GetAlbedo(ray, ray.normal) * scene->GetShade(ray);
#else
		float3 bias = 0.001f * N;
		float3 rayOrigin = I + bias;

		float3 illumination = 0;

		for (int i = 0; i < NUM_SAMPLES; ++i)
		{
			float p = 0.5f;
			if (RandomFloat() >= p)
				return illumination;

			illumination *= (1 / p);
			float3 randomUnitVec = CosineSampleHemisphere(N);
			Ray newRay = Ray(rayOrigin, randomUnitVec);
			float3 incoming = Trace(newRay, depth + 1);
			float3 BRDF = scene->GetAlbedo(ray, N) * INVPI;
			float3 cos_i = incoming * dot(randomUnitVec, N); // irradiance
			illumination += 2.f * PI * cos_i * BRDF;
		}
		
		illumination /= NUM_SAMPLES;

		return illumination;
#endif
	}
	else if (ray.objMaterial.type == MaterialType::MIRROR)
	{
		float3 bias = 0.001f * N;
		bool outside = dot(ray.D, N) < 0;

		float3 reflVec = reflect(ray.D, N);
		float3 reflRayOrigin = outside ? I + bias : I - bias;
		Ray reflRay = Ray(reflRayOrigin, normalize(reflVec));

		float3 reflectionColour = Trace(reflRay, depth + 1);

		return reflectionColour;
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

		return (fresneleffect * reflectionColour + 
				(1 - fresneleffect) * refractionColour * ray.objMaterial.Ks)
				* scene->GetAlbedo(ray, N);
	}
	else if (ray.objMaterial.type == MaterialType::LIGHT)
	{
		return BRIGHT;
	}

	return 0;
}

float3 Renderer::Sample(Ray& ray)
{
	float3 throughput = float3(1);
	float3 radiance = float3(0);

	float p = 0.5f;
	for (int i = 0; i < MAX_DEPTH; i++)
	{
		scene->FindNearest(ray);
		float3 I = ray.IntersectionPoint();
		float3 N = scene->GetNormal(ray);
		float3 BRDF = scene->GetAlbedo(ray, N) * INVPI;
		if (ray.objIdx == -1) return scene->GetSkydomeTexture(ray); // NO HIT
		if (ray.objMaterial.type == MaterialType::LIGHT) return BRIGHT; // LIGHT HIT


		float3 PL, NL;
		float A;
		scene->RandomPointOnLight(PL, NL, A);
		float3 L = normalize(PL - I);
		float3 bias = 0.001f * N;
		float3 O = ray.IntersectionPoint() + bias;
		Ray lightRay(O, L);

		if (dot(N, L) > 0 && dot(NL, -L) > 0)
		{
			scene->FindNearest(lightRay);
			if (lightRay.objMaterial.type == MaterialType::LIGHT)
			{
				float solidAngle = (dot(NL, -L) * A) / (lightRay.t * lightRay.t);
				float lightPDF = 1 / solidAngle;
				radiance += throughput * (dot(N, L) / lightPDF) * BRDF * 1.0f;
			}
		}

		// Russian Roulette
		if (RandomFloat() > p) break;
		throughput *= 1 / p;

		// Continue random walk
		float3 R = CosineSampleHemisphere(N);
		float hemiPDF = 1 / (2.f * PI);
		ray = Ray(O, R);
		throughput *= (dot(N, R) / hemiPDF) * BRDF;
	}

	return radiance;
}

float Renderer::RussianRoulette(const float3& illumination)
{
	float random = RandomFloat();
	float p = min(max(illumination.x, max(illumination.y, illumination.z)), 1.f);
	if (random > p)
	{
		return 0;
	}

	return 1/p;
}

float3 Renderer::PhotonTrace(Ray& ray, int depth)
{
	scene->FindNearest(ray);

	if (ray.objIdx == -1 || depth >= MAX_DEPTH) return BLACK;

	if (ray.objMaterial.type == MaterialType::LIGHT) return BRIGHT;
	
	if (ray.objMaterial.type == MaterialType::DIFFUSE)
	{
		if (depth >= GATHER_DEPTH)
		{
			return scene->GetRadianceFromPhotonMap(ray);
		}
		const float3 Ld = scene->GetDirectIllumination(ray);
		const float3 Lc = scene->GetCausticsIllumination();
		const float3 Li = scene->GetIndirectIllumination();
		return (Ld + Lc + Li);
	}
}

float3 Renderer::SampleHemisphere(const float3& N)
{
	float r1 = RandomFloat();
	float r2 = RandomFloat();
	float sinTheta = sqrtf(1 - r1 * r1);
	float phi = 2 * PI * r2;
	float x = sinTheta * cosf(phi);
	float z = sinTheta * sinf(phi);
	float3 randomVec = float3(x, r1, z);
	if (dot(randomVec, N) < 0) randomVec = -randomVec;
	return randomVec;
}

float3 Renderer::CosineSampleHemisphere(const float3& N)
{
	float r1 = 2.0f * PI * RandomFloat();
	float r2 = RandomFloat();
	float r2s = sqrt(r2);

	float3 w = N;
	float3 u;
	if (fabs(w.x) > 0.1f)
		u = cross(float3(0.f, 1.f, 0.f), w);
	else
		u = cross(float3(1.f, 0.f, 0.f), w);

	u = normalize(u);
	float3 v = cross(w, u);
	float3 d = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2));
	d = normalize(d);

	return d;
}

float3 Renderer::GenerateRandomVec(const float3& N)
{
	float r1 = RandomFloat();
	float r2 = RandomFloat();
	float r3 = RandomFloat();
	float3 randomVec = float3(r1, r2, r3);

	return randomVec;
}

float3 Renderer::GetSpecularColour(Light* light, const float3& I, const float3& N, const float3& D)
{
	float3 distToLight = normalize(light->GetPosition() - I);
	float A = 4 * PI * dot(distToLight, distToLight);
	float3 B = light->GetColour() / A;
	float3 reflected = normalize(reflect(-distToLight, N));
	return pow(-dot(reflected, D), 20.0f);
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Renderer::Tick( float deltaTime )
{
	data->UpdateFrameNumber();
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
			accumulator[x + y * SCRWIDTH] += float4(Sample(primaryRay), 0);
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
	data->UpdateFPS(fps);
}

void Renderer::KeyUp(int key) {}

void Renderer::KeyDown(int key) 
{
	if (key == 67) // C key pressed
	{
		float3 pos = float3(1.730125, -0.081414, 3.563606);
		camera.SetPosition(pos);
		memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
		accumulatorCounter = 0;
	}
	if (key == 87) //w
	{
		memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
		accumulatorCounter = 0;
		camera.Translate(0, 0, movespeed);
	}
	if (key == 83) //s
	{
		memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
		accumulatorCounter = 0;
		camera.Translate(0, 0, -movespeed);
	}
	if (key == 65) //a
	{
		memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
		accumulatorCounter = 0;
		camera.Translate(-movespeed, 0, 0);
	}
	if (key == 68) //d
	{
		memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
		accumulatorCounter = 0;
		camera.Translate(movespeed, 0, 0);
	}
	if (key == 81) //q
	{
		memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
		accumulatorCounter = 0;
		camera.Translate(0, movespeed, 0);
	}
	if (key == 69) //e
	{
		memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
		accumulatorCounter = 0;
		camera.Translate(0, -movespeed, 0);
	}
	if (key == 75)//k
	{
		data->PrintData("Data.csv");
	}
}

void Renderer::KeyStillDown(int key)
{
	if (key == 87) //w
	{
		memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
		accumulatorCounter = 0;
		camera.Translate(0, 0, movespeed);
	}
	if (key == 83) //s
	{
		memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
		accumulatorCounter = 0;
		camera.Translate(0, 0, -movespeed);
	}
	if (key == 65) //a
	{
		memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
		accumulatorCounter = 0;
		camera.Translate(-movespeed, 0, 0);
	}
	if (key == 68) //d
	{
		memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
		accumulatorCounter = 0;
		camera.Translate(movespeed, 0, 0);
	}
	if (key == 81) //q
	{
		memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
		accumulatorCounter = 0;
		camera.Translate(0, movespeed, 0);
	}
	if (key == 69) //e
	{
		memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
		accumulatorCounter = 0;
		camera.Translate(0, -movespeed, 0);
	}
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
		camera.Translate(-(x - prevX) * rotspeed * 0.1, (y - prevY) * rotspeed * 0.1, 0);
		/*Ray primaryRay = camera.GetPrimaryRay(x, y);
		float4 colour = float4(Trace(primaryRay, 0), 0);*/
	}
	prevX = x;
	prevY = y;
}

void Renderer::MouseWheel(float z)
{
	memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
	accumulatorCounter = 0;
	camera.Translate(0, 0, z);
}