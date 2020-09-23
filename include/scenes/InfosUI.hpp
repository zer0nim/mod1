#ifndef INFOSUI_HPP_
#define INFOSUI_HPP_

#define UI_FONT	UI_DEF_TEXT_FOND
#define UI_PAUSE_FONT	"pause"
#define UI_FONT_SCALE	1
#define UI_TEXT_COLOR	glm::vec4(.978, .97, .967, 1)

#define UPDATE_DEBUG_DATA_MS	300

#include <chrono>

#include "AUserInterface.hpp"
#include "TextInputUI.hpp"
class Scene;  // to avoid inclusion loop

/**
 * @brief debug menu to show fps at the top of the screen
 */
class InfosUI : public AUserInterface {
	public:
		// Constructors
		InfosUI(Gui & gui, Scene const & scene);
		virtual ~InfosUI();
		InfosUI(InfosUI const &src);
		InfosUI &operator=(InfosUI const &rhs);

		// Methods
		virtual bool	init();
		virtual bool	update(float dtTime);

	private:
		InfosUI();

		Scene	const & _scene;  /**< Scene reference */
		std::chrono::milliseconds	_lastUpdateMs;  /**< Last time fps was updated */
		uint16_t	_fps;  /**< Actual FPS */
		TextUI *	_fpsText;
		TextUI *	_mapText;
		TextUI *	_scenarioText;
		RectUI *	_pauseRect;
		TextUI *	_pauseText;
		TextUI *	_pauseTextKey;
};

#endif  // INFOSUI_HPP_
