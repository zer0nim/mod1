#pragma once

#define C_VAO_WIDTH 4
#define C_NB_FACES 6
// C_VAO_WIDTH * C_NB_FACES
#define C_FACE_A_SIZE 24
#define CAM_POS_OFFSET glm::vec3(0.0f, 20.0f, -10.0f)
#define CAM_TARG_OFFSET glm::vec3(0.0f, 1.0f, 0.0f)
#define CAM_SPEED 1.5f
#define VOID_POS glm::ivec2 {-1, -1}
#define VOID_POS3 glm::vec3 {-1, -1, -1}

#include <iostream>
#include <array>

#include "mod1.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Skybox.hpp"
#include "debug.hpp"
#include "Inputs.hpp"
#include "TextureManager.hpp"
#include "ABaseUI.hpp"

/**
 * @brief General information about the game
 */
struct GameInfo {
	std::string	title;  /**< Win title */
	glm::ivec2	windowSize;  /**< Win size */
	glm::ivec2	maxWindowSize;  /**< Max win size (screen size) */
	glm::ivec2	savedWindowSize;  /**< Saved win size (for next program opening) */
	bool		isSavedFullscreen;  /**< True if we want fullscreen in next program opening */
	bool		quit;  /**< True if we want to quit game */
};

/**
 * @brief This is the GUI class with function to init window, draw, ...
 */
class Gui {
	public:
		Gui();
		virtual ~Gui();
		Gui(Gui const &src);
		Gui &operator=(Gui const &rhs);

		bool	init();
		void	update();
		void	draw();
		void	preDraw();
		void	postDraw();
		void	drawSkybox(glm::mat4 &view);
		void	enableCursor(bool enable = true);
		void	drawCube(Block::Enum typeBlock, glm::vec3 pos = {0, 0, 0}, glm::vec3 scale = {1, 1, 1});
		void	updateFullscreen();
		void	updateDimension();
		bool	getWindowsFlag(uint32_t flag);
		Skybox const & getSkybox() const;

		GameInfo		gameInfo;  /**< GameInfo object */
		TextureManager	*textureManager;  /**< TextureManager object */
		Shader			*cubeShader;  /**< CubeShader object */
		Camera			*cam;  /**< Camera object */

		uint32_t		cubeShVao;  /**< Vertex Array Objects */
		uint32_t		cubeShVbo;  /**< Vertex Buffer Objects */

	private:
		SDL_Window		*_win;  /**< SDL window object */
		SDL_Event		*_event;  /**< SDL event object */
		SDL_GLContext	_context;  /**< SDL gl context object */

		Skybox			*_skybox;  /**< Project skybox */

		static std::array<float, C_FACE_A_SIZE> const		_cubeFaces;  /**< All cubes faces */

		bool	_init();
		bool	_initGameInfo();
		bool	_initOpengl();
		bool	_initShaders();
		bool	_getMaxSamples(int & maxSamples);

		static const int									_min_width = 800;  /**< Min screen width */
		static const int									_min_height = 600;  /**< Min screen height */
		bool	_protect_resolution();
};
