#include <unistd.h>
#include <chrono>

#include "mod1.hpp"
#include "Logging.hpp"
#include "FileUtils.hpp"
#include "Inputs.hpp"

SettingsJson s;

/**
 * @brief Init the logs
 *
 * You need to call this function only once at program startup.
 * This function init the logs functions
 */
void	initLogs() {
	// init logging
	#if DEBUG
		logging.setLoglevel(LOGDEBUG);
		logging.setPrintFileLine(LOGWARN, true);
		logging.setPrintFileLine(LOGERROR, true);
		logging.setPrintFileLine(LOGFATAL, true);
	#else
		logging.setLoglevel(LOGINFO);
	#endif
}

/**
 * @brief Check if the program is valid (right files, folders, ...)
 *
 * @return false If program is invalid
 */
bool	checkPrgm() {
	/* list of required directories */
	std::vector<std::string> requiredDirs = {
		"asset",
		"asset/textures",
		CONFIG_DIR,
	};

	/* list of required files */
	std::vector<std::string> requiredFiles = {
		s.j("fonts").j("base").s("file"),
	};

	for (auto && it : requiredDirs) {
		if (file::isDir(it) == false) {
			logErr(it << " directory doesn't exist");
			return false;
		}
	}
	for (auto && it : requiredFiles) {
		if (file::isFile(it) == false) {
			logErr(it << " file doesn't exist");
			return false;
		}
	}
	return true;
}

/**
 * @brief Create the pattern for master settings object & load settings
 *
 * @param filename the filename to read to set right values for settings
 * @return true if success
 * @return false if error
 */
bool	initSettings(std::string const & filename) {
	s.name("settings").description("main settings");

	s.add<std::string>("name", "mod1");

	s.add<SettingsJson>("screen");
		s.j("screen").add<uint64_t>("maxFps", 60).setMin(30).setMax(120).setDescription("framerate");

	/* font */
	s.add<SettingsJson>("fonts");
		s.j("fonts").add<SettingsJson>("base");
			s.j("fonts").j("base").add<std::string>("file", "asset/font/monaco.ttf")
				.setDescription("this is the main font");
			s.j("fonts").j("base").add<uint64_t>("size", 20).setMin(5).setMax(50)
				.setDescription("default size for the text");

	/* colors */
	s.add<SettingsJson>("colors");

	// font color
	s.j("colors").add<SettingsJson>("font");
		s.j("colors").j("font").add<uint64_t>("color", 0x181818).setMin(0x000000).setMax(0xFFFFFF);
		s.j("colors").j("font").add<uint64_t>("alpha", 0xFF).setMin(0x00).setMax(0xFF);

	// background color
	s.j("colors").add<SettingsJson>("background");
		s.j("colors").j("background").add<uint64_t>("color", 0x181818).setMin(0x000000).setMax(0xFFFFFF);
		s.j("colors").j("background").add<uint64_t>("alpha", 0xFF).setMin(0x00).setMax(0xFF);

	// collider
	s.j("colors").add<SettingsJson>("collider");
		s.j("colors").j("collider").add<uint64_t>("color", 0x647BCE).setMin(0x000000).setMax(0xFFFFFF);
		s.j("colors").j("collider").add<uint64_t>("alpha", 0xFF).setMin(0x00).setMax(0xFF);
	s.j("colors").add<SettingsJson>("boundingBox");
		s.j("colors").j("boundingBox").add<uint64_t>("color", 0x155c2c).setMin(0x000000).setMax(0xFFFFFF);
		s.j("colors").j("boundingBox").add<uint64_t>("alpha", 0xFF).setMin(0x00).setMax(0xFF);

	/* Graphics */
	s.add<SettingsJson>("graphics");
	s.j("graphics").add<bool>("fullscreen", false).setDescription("Display the game on fullscreen or not.");
	s.j("graphics").add<bool>("fitToScreen", false).setDescription("The resolution fit to the screen size");
	s.j("graphics").add<int64_t>("width", 1200).setMin(800).setMax(2560).setDescription("The resolution's width.");
	s.j("graphics").add<int64_t>("height", 800).setMin(600).setMax(1440).setDescription("The resolution's height.");

	/* mouse sensitivity */
	s.add<double>("mouse_sensitivity", 0.7).setMin(0.0).setMax(3.0) \
		.setDescription("Camera mouse sensitivity.");

	try {
		if (file::isDir(filename)) {
			logWarn(filename << " is not the settings file, it is a directory");
		}
		else if (s.loadFile(filename) == false) {
			// warning when loading settings
			return false;
		}
	}
	catch(SettingsJson::SettingsException const & e) {
		// file doesn't exist, create it at the end
		logDebug("the file " << filename << " doesn't exist for now");
		return false;
	}
	return true;
}

/**
 * @brief Save the user data
 *
 * @param filename The file to save user data
 * @return true if success
 * @return false if error
 */
bool	saveSettings(std::string const & filename) {
	try {
		if (file::isDir(filename)) {
			logWarn("cannot write in " << filename << " -> it is a directory");
		}
		s.saveToFile(filename);
	}
	catch(SettingsJson::SettingsException const & e) {
		logErr(e.what());
		return false;
	}
	return true;
}

/**
 * @brief Show usage for program
 *
 * @return false Return always false
 */
bool	usage() {
	std::cout << "usage: ./mod1 <map1.mod1> <map2.mod1> ..." << std::endl;
	return false;
}

bool	hasSuffix(std::string const & str, std::string const & suffix) {
	return str.size() >= suffix.size() &&
		str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

/**
 * @brief Parse args for program
 *
 * @param nbArgs number of arguments (argc - 1)
 * @param args arguments (av + 1)
 * @return false If need to quit
 */
bool	argParse(int nbArgs, char const ** args, std::vector<std::string> & mapsPath) {
	for (int i = 0; i < nbArgs; ++i) {
		if (strcmp(args[i], "--usage") == 0 || strcmp(args[i], "-u") == 0) {
			return usage();
		}
		else if (hasSuffix(std::string(args[i]), ".mod1")) {
			mapsPath.push_back(std::string(args[i]));
		}
		else {
			std::cout << "invalid argument: " << args[i] << std::endl;
			return usage();
		}
	}

	// we need at least one map
	if (mapsPath.size() == 0)
		return usage();

	return true;
}

/**
 * @brief Get the current time in ms
 *
 * @return std::chrono::milliseconds the ms object
 */
std::chrono::milliseconds getMs() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch());
}

/**
 * @brief Transform color between hexadecimal mode to float vec4.
 *
 * @param color
 * @param alpha
 * @return glm::vec4
 */
glm::vec4					colorise(uint32_t color, uint8_t alpha) {
	float	red = static_cast<float>((color & 0xFF0000) >> 16) / 255.0;
	float	green = static_cast<float>((color & 0x00FF00) >> 8) / 255.0;
	float	blue = static_cast<float>(color & 0x0000FF) / 255.0;
	if (color > 0xffffff) {
		red = 1.0;
		green = 1.0;
		blue = 1.0;
	}
	return (glm::vec4({red, green, blue, (alpha / 255.0)}));
}
