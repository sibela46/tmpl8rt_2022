// Template, IGAD version 3
// IGAD/NHTV/UU - Jacco Bikker - 2006-2022

// common.h is to be included in host and device code and stores
// global settings and defines.

// constants
#define PI				3.14159265358979323846264f
#define E				2.71828
#define INVPI			0.31830988618379067153777f
#define INV2PI			0.15915494309189533576888f
#define TWOPI			6.28318530717958647692528f
#define SQRT_PI_INV		0.56418958355f
#define LARGE_FLOAT		1e34f
#define EPSILON			0.0000001
#define BLACK			float3(0, 0, 0)
#define WHITE			float3(1, 1, 1)
#define GREY			float3(0.5, 0.5, 0.5)
#define BRIGHT			float3(1, 1, 1)
#define RED				float3(1.0, 0.1, 0.1)
#define GREEN			float3(0.1, 0.9, 0.1)
#define BLUE			float3(0, 0.2118, 0.6392)
#define PURPLE			float3(0.8118, 0.0078, 0.4275)
#define HORIZON			10.f
#define NUM_PHOTONS		100
#define MAX_DEPTH		5
#define GATHER_DEPTH	10
#define K				5
#define NUM_SAMPLES		3

// IMPORTANT NOTE ON OPENCL COMPATIBILITY ON OLDER LAPTOPS:
// Without a GPU, a laptop needs at least a 'Broadwell' Intel CPU (5th gen, 2015):
// Intel's OpenCL implementation 'NEO' is not available on older devices.
// Same is true for Vulkan, OpenGL 4.0 and beyond, as well as DX11 and DX12.