#pragma once

#include <iostream>
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <stb_image.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <cstdlib>
#include <chrono>

#include "FileUtils.hpp"
#include "SettingsJson.hpp"
#include "Logging.hpp"
#include "useGlm.hpp"
#include "debug.hpp"

#ifndef DEBUG
	#define DEBUG !NDEBUG
#endif
/* print log when fps level is too low */
#define DEBUG_FPS_LOW	DEBUG & false  // always false in normal mode
/* open the exit menu before quitting */
#define ASK_BEFORE_QUIT	!DEBUG | true  // always true in normal mode
/* show help (shortcuts in buttons) */
#define DEBUG_SHOW_HELP	DEBUG & true  // always false in normal mode

#define CONFIG_DIR				"configs/"

#define SETTINGS_FILE			CONFIG_DIR"settings.json"
#define CONTROLS_FILE			CONFIG_DIR"controls.json"

#define MAX_POINTS_NB 50
#define BOX_MAX_SIZE glm::uvec3(32, 128, 32)

void	initLogs();
bool	checkPrgm();
bool	initSettings(std::string const & filename);
bool	saveSettings(std::string const & filename);
bool	usage();
bool	hasSuffix(std::string const & str, std::string const & suffix);
bool	argParse(int nbArgs, char const ** args, std::vector<std::string> & mapsPath);
std::chrono::milliseconds	getMs();
glm::vec4	colorise(uint32_t color, uint8_t alpha = 0xff);

/**
 * @brief global variable for general settings
 */
extern SettingsJson s;
