#include "gizmo_example.h"
#include "terrain_example.h"
#include "water_example.h"
#include "atmosphere_example.h"
#include "pbr_example.h"

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
	Register("Gizmo", CreateGizmoExampleFn);
	Register("Terrain", CreateTerrainExampleFn);
	Register("Water", CreateWaterExampleFn);
	Register("Atmosphere", CreateAtmosphereExampleFn);
	Register("PBR", CreatePBRExampleFn);

	ExampleBase* example = GetExample("Terrain");
	if (example)
	{
		example->run();
		delete example;
	}
}
