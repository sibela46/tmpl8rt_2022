#pragma once

namespace Tmpl8
{

	class Renderer : public TheApp
	{
	public:
		// game flow methods
		void Init();
		float3 Trace(Ray& ray, int depth);
		float3 Sample(Ray& ray);
		float3 PhotonTrace(Ray& ray, int depth);
		float3 GenerateRandomVec(const float3& N);
		float3 SampleHemisphere(const float3& N);
		float3 CosineSampleHemisphere(const float3& N);
		float3 GetSpecularColour(Light* light, const float3& I, const float3& N, const float3& D);
		float RussianRoulette(const float3& illumination);
		void Tick(float deltaTime);
		void Shutdown() { /* implement if you want to do something on exit */ }
		// input handling
		void MouseUp(int button); /* implement if you want to detect mouse button presses */
		void MouseDown(int button); /* implement if you want to detect mouse button presses */
		void MouseMove(int x, int y);
		void MouseWheel(float z); /* implement if you want to handle the mouse wheel */
		void KeyUp(int key); /* implement if you want to handle keys */
		void KeyStillDown(int key); /* implement if you want to handle keys */
		void KeyDown(int key); /* implement if you want to handle keys */
	// data members
	int2 mousePos;
	float4* accumulator;
	int accumulatorCounter = 0;
	//Scene scene;
	Scene* scene;
	DataCollector* data;
	Camera camera;
	bool pressR;
	bool pressL;
	float rotspeed = 0.1;
	float movespeed = 0.1;
	int prevX;
	int prevY;
	bool forward;
};

} // namespace Tmpl8