#pragma once

namespace Tmpl8
{

class Renderer : public TheApp
{
public:
	// game flow methods
	void Init();
	float3 Trace( Ray& ray, int depth );
	float3 Shade();
	void Tick( float deltaTime );
	void Shutdown() { /* implement if you want to do something on exit */ }
	// input handling
	void MouseUp( int button ) { /* implement if you want to detect mouse button presses */ 
		if (button == 0)
			press = false;
		if (button == 1)
			pressL = false;
	}
	void MouseDown( int button ) { /* implement if you want to detect mouse button presses */ 
		if(button==0)
			press = true;
		if (button == 1)
			pressL = true;
	}
	void MouseMove( int x, int y ) { mousePos.x = x, mousePos.y = y;
		if (pressL) {
			camera.RotateX((y - prevY) * rotspeed);
			camera.RotateY((x - prevX) * rotspeed);
		}
		if (press) {
			camera.Translate(-(x - prevX) * rotspeed * 0.05, (y - prevY) * rotspeed * 0.05, 0);
		}
		prevX = x;
		prevY = y;
	}
	void MouseWheel( float y ) { /* implement if you want to handle the mouse wheel */ 
		camera.Translate(0, 0, y);
	}
	void KeyUp( int key ) { /* implement if you want to handle keys */ }
	void KeyDown( int key ) { /* implement if you want to handle keys */ }
	void KeyPress();
	// data members
	int2 mousePos;
	float4* accumulator;
	//Scene scene;
	Scene* scene;
	Camera camera;
	bool press;
	bool pressL;
	float rotspeed = 0.3;
	int prevX;
	int prevY;
};

} // namespace Tmpl8