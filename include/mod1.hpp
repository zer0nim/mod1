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

#define CONFIG_DIR				"configs/"

#define SETTINGS_FILE			CONFIG_DIR"settings.json"
#define CONTROLS_FILE			CONFIG_DIR"controls.json"

#define MAX_POINTS_NB 50
#define BOX_MAX_SIZE glm::vec3(64, 64, 64)
#define BOX_GROUND_HEIGHT 24

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
