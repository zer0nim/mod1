#include <vector>
#include "mod1.hpp"
#include "Terrain.hpp"
#include "Gui.hpp"
#include "Scene.hpp"

bool	init(int ac, char const **av, std::vector<Terrain *> & terrains) {
	std::vector<std::string>	mapsPath;

	initLogs();  // init logs functions
	srand(time(NULL));  // init random

	if (!argParse(ac - 1, av + 1, mapsPath))  // parse arguments
		return false;
	// create Terrain object for each file argument
	try {
		for (std::string mapPath : mapsPath) {
			terrains.push_back(new Terrain(mapPath));
		}
	} catch(Terrain::TerrainException const & e) {
		logErr(e.what());
		return false;
	}

	file::mkdir(CONFIG_DIR);  // create config folder
	initSettings(SETTINGS_FILE);  // create settings object

	return checkPrgm();
}

bool	simulation(std::vector<Terrain *> & terrains) {
	Scene	scene;

	// test calculateHeight function
	for (Terrain * & terrain : terrains) {
		logDebug("");
		for (uint16_t v = 0; v < BOX_MAX_SIZE.z; ++v) {
			for (uint16_t u = 0; u < BOX_MAX_SIZE.x; ++u) {
				glm::uvec2 pos(u, v);
				logDebug("calculateHeight( " << glm::to_string(pos) << " ) -> " <<
					std::to_string(terrain->calculateHeight(pos)));
			}
		}
		logDebug("");
	}

	if (!scene.init()) {
		return false;
	}

	return scene.run();
}

int main(int ac, char const **av) {
	int	ret = EXIT_SUCCESS;
	std::vector<Terrain *>	terrains;

	// init program & load settings
	if (!init(ac, av, terrains))
		ret = EXIT_FAILURE;

	if (ret != EXIT_FAILURE) {
		// launch simulation
		ret = simulation(terrains);

		// save settings before exiting
		if (ret != EXIT_FAILURE)
			saveSettings(SETTINGS_FILE);
	}

	// free variables
	for (Terrain * t : terrains)
		delete t;

	return ret;
}
