#include <vector>
#include "mod1.hpp"
#include "Terrain.hpp"
#include "Gui.hpp"
#include "Scene.hpp"

bool	init(int ac, char const **av, Scene & scene, std::vector<Terrain *> & terrains) {
	std::vector<std::string>	mapsPath;

	initLogs();  // init logs functions
	srand(time(NULL));  // init random

	file::mkdir(CONFIG_DIR);  // create config folder
	initSettings(SETTINGS_FILE);  // create settings object

	if (!scene.init()) {
		return false;
	}

	if (!argParse(ac - 1, av + 1, mapsPath))  // parse arguments
		return false;
	// create Terrain object for each file argument
	try {
		for (std::string mapPath : mapsPath) {
			terrains.push_back(new Terrain(mapPath, scene.getGui()));
		}
	} catch(Terrain::TerrainException const & e) {
		logErr(e.what());
		return false;
	}

	return checkPrgm();
}

bool	simulation(std::vector<Terrain *> & terrains, Scene &scene) {
	// test calculateHeight function
	for (Terrain * & terrain : terrains) {
		if (!terrain->initMesh())
			return false;
	}

	return scene.run();
}

int main(int ac, char const **av) {
	int	ret = EXIT_SUCCESS;
	std::vector<Terrain *>	terrains;
	Scene	scene(terrains);

	// init program & load settings
	if (!init(ac, av, scene, terrains))
		ret = EXIT_FAILURE;

	if (ret != EXIT_FAILURE) {
		// launch simulation
		ret = simulation(terrains, scene);

		// save settings before exiting
		if (ret != EXIT_FAILURE)
			saveSettings(SETTINGS_FILE);
	}

	// free variables
	for (Terrain * t : terrains)
		delete t;

	return ret;
}
