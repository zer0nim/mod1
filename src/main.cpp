#include <vector>
#include "mod1.hpp"
#include "Terrain.hpp"

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

int main(int ac, char const **av) {
	int	ret = EXIT_SUCCESS;
	std::vector<Terrain *>	terrains;

	// init program & load settings
	if (!init(ac, av, terrains))
		ret = EXIT_FAILURE;

	if (ret != EXIT_FAILURE) {
		// save settings before exiting
		saveSettings(SETTINGS_FILE);
	}

	// free variables
	for (Terrain * t : terrains)
		delete t;

	return ret;
}
