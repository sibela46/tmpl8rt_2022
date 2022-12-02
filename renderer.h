#pragma once

namespace Tmpl8
{

	class Renderer : public TheApp
	{
	public:
		// game flow methods
		void Init();
		float3 Trace(Ray& ray, int depth);
		void Tick(float deltaTime);
		void Shutdown() { /* implement if you want to do something on exit */ }
		// input handling
		void MouseUp(int button); /* implement if you want to detect mouse button presses */
		void MouseDown(int button); /* implement if you want to detect mouse button presses */
		void MouseMove(int x, int y);
		void MouseWheel(float z); /* implement if you want to handle the mouse wheel */
		void KeyUp(int key) { /* implement if you want to handle keys */ }
		void KeyDown(int key); /* implement if you want to handle keys */
	// data members
	int2 mousePos;
	float4* accumulator;
	//Scene scene;
	Scene* scene;
	Camera camera;
	bool pressR;
	bool pressL;
	float rotspeed = 0.2;
	int prevX;
	int prevY;
};

} // namespace Tmpl8