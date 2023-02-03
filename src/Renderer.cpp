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
	sampler = new GuidedSampler();
	scene = new Scene(data);
}

// -----------------------------------------------------------
// Evaluate light transport
// -----------------------------------------------------------
float3 Renderer::Trace( Ray& ray, int depth )
{
	scene->FindNearest(ray);
	if (ray.objIdx == -1) return scene->GetSkydomeTexture(ray); // or a fancy sky color
	if (depth >= MAX_DEPTH) return BLACK;

	if (depth == 0) data->UpdateIntersectedPrimitives();

	float3 I = ray.O + ray.t * ray.D;
	float3 N = scene->GetNormal(ray);

	float3 colour = 0;

	float3 albedo = scene->GetAlbedo(ray, N);
	if (ray.objMaterial.type == MaterialType::SAMPLEPOINT) return BLACK;

	if (ray.objMaterial.type == MaterialType::DIFFUSE)
	{
#ifdef WHITTED_STYLE
		return scene->GetAlbedo(ray, ray.normal) * scene->GetShade(ray);
#else

		float3 illumination = 0;

		for (int i = 0; i < NUM_SAMPLES; ++i)
		{
			float weightIndex = -1;
#ifdef PATH_GUIDING
			float3 randomUnitVec = sampler->SampleDirection(I, N, weightIndex);
#else
			float3 randomUnitVec = SampleHemisphere(N);
#endif // PATH_GUIDING

			float3 bias = 0.001f * N;
			float3 rayOrigin = I + bias;
			Ray newRay = Ray(rayOrigin, randomUnitVec);
			float nDotR = dot(N, randomUnitVec);
			float3 BRDF = albedo * INVPI;
			float3 PDF = 1 / (2 * PI);
			illumination += Trace(newRay, depth+1) * nDotR * BRDF * (1 / PDF);

#ifdef PATH_GUIDING
			sampler->UpdateWeights(I, weightIndex, illumination);
#endif // PATH_GUIDING
		}
		
		illumination /= NUM_SAMPLES;

		colour = illumination;

#ifdef RUSSIAN_ROULETTE
		float p = max(albedo.x, max(albedo.y, albedo.z));
		if (p < RandomFloat())
			return colour;

		colour *= (1 / p);
#endif // RUSSIAN_ROULETTE

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

		colour = reflectionColour;
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

		colour = (fresneleffect * reflectionColour + 
				(1 - fresneleffect) * refractionColour * ray.objMaterial.Ks)
				* scene->GetAlbedo(ray, N);
	}
	else if (ray.objMaterial.type == MaterialType::LIGHT)
	{
		connectedLightRays++;
		colour = BRIGHT;
	}

	return colour;
}

void Renderer::TraceGuidingPath(Ray& ray, int depth)
{
	scene->FindNearest(ray);
	if (ray.objIdx == -1) return; // or a fancy sky color

	switch (ray.objMaterial.type)
	{
	case MaterialType::DIFFUSE:
		{
			float3 I = ray.O + ray.t * ray.D;
			float3 N = scene->GetNormal(ray);

			float3 bias = 0.001f * N;
			float3 rayOrigin = I + bias;

			for (int i = 0; i < NUM_SAMPLES; ++i)
			{
				float3 randomUnitVec = SampleHemisphere(N);
				Ray newRay = Ray(rayOrigin, randomUnitVec);
				
				bool reject = sampler->RejectIfInRadius(I, 0.01f);
				if (!reject)
				{
					sampler->AddSamplePoint(I, randomUnitVec);
				}
				// diffuse bounce
				TraceGuidingPath(newRay, depth + 1);
			}
		}
		break;
	case MaterialType::LIGHT:
		{
			return;
		}
		break;
	default:
		break;
	}
}

void Renderer::BuildGuidingPath()
{
	if (!isTrainingPhase) return;
	// build SD-tree
	Ray centreRay = camera.GetPrimaryRay(SCRWIDTH / 2, SCRHEIGHT / 2);
	for (int i = 0; i < 1000; i++)
	{
		TraceGuidingPath(centreRay, 0);
	}
	sampler->AddPath();
	//scene->AddSamplePointsToScene(sampler->GetSamplePoints());
}

void Renderer::StopTrainingPhase()
{
	isTrainingPhase = false;
}

int Renderer::GetConnectedLightRays()
{
	return connectedLightRays;
}

float3 Renderer::Sample(Ray& ray)
{
	float3 throughput = float3(1);
	float3 radiance = float3(0);

	float p = 0.5f;
	for (int i = 0; i < MAX_DEPTH; i++)
	{
		scene->FindNearest(ray);
		if (ray.objIdx == -1) return BLACK;// scene->GetSkydomeTexture(ray); // NO HIT

		float3 I = ray.IntersectionPoint();
		float3 N = scene->GetNormal(ray);
		if (ray.objMaterial.type == MaterialType::LIGHT)
		{
			return BRIGHT; // LIGHT HIT
		}

		float3 PL, NL;
		float A;
		scene->RandomPointOnLight(PL, NL, A);
		float3 L = normalize(PL - I);
		float3 bias = 0.001f * L;
		float3 O = ray.IntersectionPoint() + bias;
		Ray lightRay(O, L);

		float3 albedo = scene->GetAlbedo(ray, N);
		float3 BRDF = albedo * INVPI;

		if (dot(N, L) > 0 && dot(NL, -L) > 0)
		{
			scene->FindNearest(lightRay);
			if (lightRay.objMaterial.type == MaterialType::LIGHT)
			{
				float solidAngle = (dot(NL, -L) * A) / (lightRay.t * lightRay.t);
				float lightPDF = 1 / solidAngle;
				radiance += throughput * (dot(N, L) / lightPDF) * BRDF;
			}
		}

		// Russian Roulette
		//float p = max(throughput.x, max(throughput.y, throughput.z));

		// Continue random walk
		float3 R;
		if (ray.objMaterial.type == MaterialType::MIRROR)
		{
			R = reflect(ray.D, N);
			O = ray.IntersectionPoint() + 0.001f * R;
			ray = Ray(O, R);
		}
		else if (ray.objMaterial.type == MaterialType::GLASS)
		{
			float fresneleffect = fresnel(I, N, 1.5);
			if (RandomFloat() < fresneleffect)
			{
				R = reflect(ray.D, N);
				bias = 0.001f * R;
				O = ray.IntersectionPoint() + bias;
				ray = Ray(O, R);
			}
			else
			{
				bool outside = dot(ray.D, N) < 0;
				float ior = 1.5, eta = (outside) ? 1 / ior : ior;
				float cosi = -dot(ray.D, N);
				float k = 1 - eta * eta * (1 - cosi * cosi);
				float3 refrdir = ray.D * eta + N * (eta * cosi - sqrt(k));
				bias = 0.001f * refrdir;
				float3 refrRayOrigin = outside ? I + bias : I - bias;
				ray = Ray(refrRayOrigin, (refrdir));
				float nDotR = dot(N, refrdir);
				float hemiPDF = nDotR / PI;
				throughput *= (nDotR / hemiPDF) * BRDF;
			}
		}
		else
		{
			if (p < RandomFloat()) break;

			throughput *= (1 / p);

			R = CosineSampleHemisphere(N);
			O = ray.IntersectionPoint() + 0.001f * R;
			ray = Ray(O, R);
			float nDotR = dot(N, R);
			float hemiPDF = nDotR / PI;
			throughput *= (nDotR / hemiPDF) * BRDF;
		}
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
	float y = sinTheta * sinf(phi);
	float3 randomVec = float3(x, y, r1);
	if (dot(randomVec, N) < 0) randomVec = -randomVec;
	return randomVec;
}

float3 Renderer::CosineSampleHemisphere(const float3& N)
{
	float r1 = RandomFloat(), r0 = RandomFloat();
	float r = sqrt(r0);
	float theta = 2 * PI * r1;
	float x = r * cosf(theta);
	float y = r * sinf(theta);
	float3 dir = normalize(float3(x, y, sqrt(1 - r0)));
	return  dot(dir, N) > 0.f ? dir : dir * -1;
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
#ifdef NEE
			accumulator[x + y * SCRWIDTH] += float4(Sample(primaryRay), 0);
#else
			accumulator[x + y * SCRWIDTH] += float4(Trace(primaryRay, 0), 0);
#endif
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
	
	if (accumulatorCounter >= TRAINING_COUNT)
	{
		StopTrainingPhase();
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