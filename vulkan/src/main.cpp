#include "cube_example.h"
#include "gizmo_example.h"
#include "terrain_example.h"
#include "water_example.h"
#include "atmosphere_example.h"

#include <functional>
#include <map>

std::map<std::string, std::function<ExampleBase*()>> g_Tests;

void Register(std::string name, std::function<ExampleBase*()> createFunc)
{
	g_Tests[name] = createFunc;
}

ExampleBase* GetExample(std::string name)
{
	return g_Tests[name]();
}

int main()
{
	Register("CubeExample", CreateCubeExampleFn);
	Register("GizmoExample", CreateGizmoExampleFn);
	Register("TerrainExample", CreateTerrainExampleFn);
	Register("WaterExample", CreateWaterExampleFn);
	Register("AtmosphereExample", CreateAtmosphereExampleFn);

	ExampleBase* example = GetExample("AtmosphereExample");
	if (example)
	{
		example->run();
		delete example;
	}
}
