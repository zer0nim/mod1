#include <iostream>
#include <string>

#include "InfosUI.hpp"
#include "Scene.hpp"

/**
 * @brief Construct a new Scene Debug:: Scene Debug object
 *
 * @param gui A pointer on the gui object
 */
InfosUI::InfosUI(Gui & gui, Scene const & scene, UiState & uiState)
: AUserInterface(gui),
  _scene(scene),
  _uiState(uiState)
{
	_lastUpdateMs = getMs();
	_fps = _scene.getFps();
}

/**
 * @brief Destroy the Scene Debug:: Scene Debug object
 */
InfosUI::~InfosUI() {
}

/**
 * @brief Construct a new Scene Debug:: Scene Debug object
 *
 * @param src The object to do the copy
 */
InfosUI::InfosUI(InfosUI const &src)
: AUserInterface(src),
  _scene(src._scene),
  _uiState(src._uiState) {
	*this = src;
}

/**
 * @brief Copy this object
 *
 * @param rhs The object to copy
 * @return InfosUI& A reference to the copied object
 */
InfosUI &InfosUI::operator=(InfosUI const &rhs) {
	if (this != &rhs) {
		logWarn("you are copying InfosUI")
	}

	return *this;
}

/**
 * @brief init the menu
 *
 * @return true if the init succeed
 * @return false if the init failed
 */
bool InfosUI::init() {
	glm::vec2 winSz = _gui.gameInfo.windowSize;
	glm::vec2 pos, size, ui, sizeTmp;
	glm::vec2 marg(4, 4);
	std::string str;

	try {
		ui.y = ABaseUI::strHeight(UI_FONT, UI_FONT_SCALE) * 1.6;

		// fps text
		str = std::to_string(_fps) + "fps";
		ui.x = ABaseUI::strWidth(UI_FONT, str, UI_FONT_SCALE) + 4;
		size = {ui.x, ui.y};
		pos = {winSz.x - marg.x - size.x, winSz.y - marg.y - ui.y};
		_fpsText = &addText(pos, size, str);
		_fpsText->setTextFont(UI_FONT)
			.setTextOutline(.17)
			.setTextScale(UI_FONT_SCALE)
			.setTextColor(UI_TEXT_COLOR)
			.setTextAlign(TextAlign::RIGHT)
			.setZ(1);

		// map text
		str = "map " + std::to_string(_scene.getTerrainId() + 1);
		ui.x = ABaseUI::strWidth(UI_FONT, str, UI_FONT_SCALE) + 4;
		size = {ui.x, ui.y};
		sizeTmp = size;
		pos = {winSz.x / 2 - size.x / 2, winSz.y - marg.y - ui.y};
		_mapText = &addText(pos, size, str);
		_mapText->setTextFont(UI_FONT)
			.setTextOutline(.17)
			.setTextScale(UI_FONT_SCALE)
			.setTextColor(UI_TEXT_COLOR)
			.setZ(1);


		// left button
		str = "<";
		ui.x = ABaseUI::strWidth(UI_FONT, str, UI_FONT_SCALE) + 12;
		size = {ui.x, ui.y};
		pos = {winSz.x / 2 - sizeTmp.x / 2 - size.x - marg.x, winSz.y - 4 - ui.y};
		addButton(pos, size, str)
			.setKeyLeftClickScancode(Inputs::getKeySdlScancode(InputType::DECREMENT_1))
			.addButtonLeftListener(&_uiState.leftBtn)
			.setTextScale(UI_FONT_SCALE)
			.setTextColor(UI_TEXT_COLOR)
			.setZ(1)
			.showHelp(true);

		// right button
		str = ">";
		pos = {winSz.x / 2 + sizeTmp.x / 2 + marg.x, winSz.y - 4 - ui.y};
		addButton(pos, size, str)
			.setKeyLeftClickScancode(Inputs::getKeySdlScancode(InputType::INCREMENT_1))
			.addButtonLeftListener(&_uiState.rightBtn)
			.setTextScale(UI_FONT_SCALE)
			.setTextColor(UI_TEXT_COLOR)
			.setZ(1)
			.showHelp(true);

		// scenario button
		str = "scenario";
		ui.x = ABaseUI::strWidth(UI_FONT, str, UI_FONT_SCALE) + 12;
		size = {ui.x, ui.y};
		sizeTmp = size;
		pos = {marg.x, winSz.y - 4 - ui.y};
		addButton(pos, size, str)
			.setKeyLeftClickScancode(Inputs::getKeySdlScancode(InputType::GOTO_MENU))
			.addButtonLeftListener(&_uiState.scenarioBtn)
			.setTextScale(UI_FONT_SCALE)
			.setTextColor(UI_TEXT_COLOR)
			.setZ(1)
			.showHelp(true);

		// scenario text
		str = Water::flowScenarioName[_scene.getScenarioId()];
		ui.x = ABaseUI::strWidth(UI_FONT, str, UI_FONT_SCALE) + 4;
		size = {ui.x, ui.y};
		pos = {sizeTmp.x + marg.x, winSz.y - marg.y - ui.y};
		_scenarioText = &addText(pos, size, str);
		_scenarioText->setTextFont(UI_FONT)
			.setTextOutline(.17)
			.setTextAlign(TextAlign::LEFT)
			.setTextScale(UI_FONT_SCALE)
			.setTextColor(UI_TEXT_COLOR)
			.setZ(1);

		// pause ui
		if (_scene.getPause()) {
			glm::vec4 pauseBgColor = colorise(0x1f212d, 0.6 * 255);
			glm::vec4 pauseOutlineColor = colorise(0x7785bf, 255);
			float boxMarg = marg.y * 8;
			float totalH = ABaseUI::strHeight(UI_PAUSE_FONT, UI_FONT_SCALE) + marg.y * 2;
			totalH += ui.y + marg.y * 2 + boxMarg;

			// background box
			str = "PAUSE";
			ui.x = ABaseUI::strWidth(UI_PAUSE_FONT, str, UI_FONT_SCALE) + marg.x;
			size = {ui.x + boxMarg, totalH};
			pos = {winSz.x / 2 - size.x / 2, winSz.y / 2 - size.y / 2};
			_pauseRect = &addRect(pos, size, pauseBgColor);
			_pauseRect->setBorderColor(pauseOutlineColor)
				.setBorderSize(.8);

			// pause text
			str = "PAUSE";
			ui.x = ABaseUI::strWidth(UI_PAUSE_FONT, str, UI_FONT_SCALE) + marg.x;
			size = {ui.x, ABaseUI::strHeight(UI_PAUSE_FONT, UI_FONT_SCALE) + marg.y * 2};
			sizeTmp = size;
			pos = {winSz.x / 2 - size.x / 2, pos.y + totalH - size.y - boxMarg / 2};
			_pauseText = &addText(pos, size, str);
			_pauseText->setTextFont(UI_PAUSE_FONT)
				.setTextOutlineColor(pauseOutlineColor)
				.setTextOutline(.105)
				.setTextScale(UI_FONT_SCALE)
				.setTextColor(glm::vec4(1, 1, 1, 0))
				.setZ(1);

			// _pauseTextKey
			str = "press " + Inputs::getKeyName(InputType::ACTION) + " to play";
			ui.x = ABaseUI::strWidth(UI_FONT, str, UI_FONT_SCALE) + 4;
			size = {ui.x, ui.y};
			pos = {winSz.x / 2 - size.x / 2, pos.y - size.y - marg.y * 2};
			_pauseTextKey = &addText(pos, size, str);
			_pauseTextKey->setTextFont(UI_FONT)
				.setTextScale(UI_FONT_SCALE)
				.setTextColor(pauseOutlineColor)
				.setZ(1);
		}
	}
	catch (ABaseUI::UIException const & e) {
		logErr(e.what());

		return false;
	}
	return true;
}

/**
 * @brief this is the update function (called every frames)
 *
 * @param dtTime delta time between last update the update is a success
 * @return true if the update is a success
 * @return false if we need to quit the command line
 */
bool InfosUI::update(float dtTime) {
	AUserInterface::update(dtTime);
	std::string str;

	// update fps
	if (getMs().count() - _lastUpdateMs.count() > UPDATE_DEBUG_DATA_MS) {
		_lastUpdateMs = getMs();

		// update fps
		_fps = _scene.getFps();
		str = std::to_string(_fps) + "fps";
		_fpsText->setText(str);
	}

	// update map
	str = "map " + std::to_string(_scene.getTerrainId() + 1);
	_mapText->setText(str);

	// update scenario
	str = Water::flowScenarioName[_scene.getScenarioId()];
	_scenarioText->setText(str);

	// update pause ui
	bool pause = _scene.getPause();
	_pauseRect->setEnabled(pause);
	_pauseText->setEnabled(pause);
	_pauseTextKey->setEnabled(pause);

	return true;
}
