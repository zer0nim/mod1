#ifndef AUSERINTERFACE_HPP_
#define AUSERINTERFACE_HPP_

#include <vector>
#include "useGlm.hpp"
#include "Gui.hpp"

/* import all UI objects */
#include "ABaseUI.hpp"
#include "ButtonUI.hpp"
#include "ButtonImageUI.hpp"
#include "SliderUI.hpp"
#include "TextUI.hpp"
#include "RectUI.hpp"
#include "ImageUI.hpp"
#include "ScrollbarUI.hpp"
#include "EmptyMasterUI.hpp"
#include "TextInputUI.hpp"

#define VOID_COLOR glm::vec4 {-1 , -1, -1, -1}

/**
 * @brief Struct TransparentBox, store transparent ui pos/size for blur effect
 */
struct	TransparentBox {
	glm::vec2	pos;  /**< The box position */
	glm::vec2	size;  /**< The box size */
};

/**
 * @brief Scene object to re-implement in all scenes for menu
 *
 * this object contains functions to create buttons, images, ...
 */
class AUserInterface {
	public:
		AUserInterface(Gui & gui);
		virtual ~AUserInterface();
		AUserInterface(AUserInterface const &src);
		AUserInterface &operator=(AUserInterface const &rhs);

		// AScene methods
		virtual bool		init() = 0;
		virtual bool		update(float dtTime);
		virtual bool		draw();

		// add element
		ButtonUI &			addButton(glm::vec2 pos, glm::vec2 size, std::string const & text);
		ButtonImageUI &		addButtonImage(glm::vec2 pos, glm::vec2 size, std::string const & filename,
			std::string const & filenameHover = "");
		SliderUI &			addSlider(glm::vec2 pos, glm::vec2 size, float min, float max, float val, float step);
		TextUI &			addText(glm::vec2 pos, glm::vec2 size, std::string const & text);
		TextUI &			addTitle(glm::vec2 pos, glm::vec2 size, std::string const & text);
		RectUI &			addRect(glm::vec2 pos, glm::vec2 size, glm::vec4 color = VOID_COLOR,
			glm::vec4 borderColor = VOID_COLOR);
		ImageUI &			addImage(glm::vec2 pos, glm::vec2 size, std::string const & filename);
		ScrollbarUI &		addScrollbar(glm::vec2 pos, glm::vec2 size);
		EmptyMasterUI &		addEmptyMaster();
		TextInputUI &		addTextInput(glm::vec2 pos, glm::vec2 size, std::string const & defText);

		ButtonImageUI &		addExitButton();

		// getter
		ABaseUI &			getUIElement(uint32_t id);
		uint32_t			getNbUIElements() const;

		// Exceptions
		/**
		 * @brief Scene exception
		 */
		class SceneException : public std::runtime_error {
		public:
			SceneException();
			explicit SceneException(const char* whatArg);
		};

	protected:
		Gui	& _gui;  /**< A reference to the gui object */
		std::vector<ABaseUI *>	_buttons;  /**< All UI elements (auto added with addXXX functions) */

		bool	_initBG();
};

#endif  // AUSERINTERFACE_HPP_
