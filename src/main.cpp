#include <vector>
#include "mod1.hpp"

bool	init(int ac, char const **av) {
	std::vector<std::string>	mapsPath;

	initLogs();  // init logs functions
	srand(time(NULL));  // init random

	if (!argParse(ac - 1, av + 1, mapsPath))  // parse arguments
		return false;

	file::mkdir(CONFIG_DIR);  // create config folder
	initSettings(SETTINGS_FILE);  // create settings object

	return checkPrgm();
}

int main(int ac, char const **av) {
	// init program & load settings
	if (!init(ac, av))
		return false;

	// save settings before exiting
	saveSettings(SETTINGS_FILE);

	return 0;
}
