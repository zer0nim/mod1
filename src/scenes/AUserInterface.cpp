#include "AUserInterface.hpp"

// -- Constructors -------------------------------------------------------------

/**
 * @brief Construct a new AUserInterface::AUserInterface object
 *
 * @param gui A pointer on the gui object
 */
AUserInterface::AUserInterface(Gui & gui)
: _gui(gui) {}

/**
 * @brief Destroy the AUserInterface::AUserInterface object
 */
AUserInterface::~AUserInterface() {
	for (auto it = _buttons.begin(); it != _buttons.end(); it++) {
		delete *it;
	}
	_buttons.clear();
}

/**
 * @brief Construct a new AUserInterface::AUserInterface object
 *
 * @param src The object to do the copy
 */
AUserInterface::AUserInterface(AUserInterface const &src)
: _gui(src._gui) {
	*this = src;
}

// -- Operators ----------------------------------------------------------------

/**
 * @brief Copy this object
 *
 * @param rhs The object to copy
 * @return AUserInterface& A reference to the copied object
 */
AUserInterface &AUserInterface::operator=(AUserInterface const &rhs) {
	if ( this != &rhs ) {
		logWarn("AUserInterface object copied");
	}
	return *this;
}

// -- Methods ------------------------------------------------------------------

/**
 * @brief this is the update function (called every frames)
 *
 * @param dtTime delta time between last update the update is a success
 * @return true if
 * @return false if there are an error in update
 */
bool	AUserInterface::update(float dtTime) {
	for (auto it = _buttons.begin(); it != _buttons.end(); it++) {
		(*it)->update();
	}
	return true;
}

/**
 * @brief this is the draw function (called every frames)
 *
 * @return true if the draw is a success
 * @return false if there are an error in draw
 */
bool	AUserInterface::draw() {
	bool ret = true;

	/* UI elements */
	for (auto it = _buttons.begin(); it != _buttons.end(); it++) {
		(*it)->draw();
	}
	return ret & true;
}

/**
 * @brief add a button in the menu with menu settings
 *
 * @param pos the position
 * @param size the size
 * @param text the text in the button
 * @return ButtonUI& a reference to the element created
 */
ButtonUI & AUserInterface::addButton(glm::vec2 pos, glm::vec2 size, std::string const & text) {
	ButtonUI * ui = new ButtonUI(pos, size);
	ui->setText(text);
	// set default color
	glm::vec4 color = colorise(s.j("colors").j("buttons").u("color"), s.j("colors").j("buttons").u("alpha"));
	ui->setColor(color);
	ui->setBorderColor(colorise(
		s.j("colors").j("buttons-border").u("color"),
		s.j("colors").j("buttons-border").u("alpha")
	));
	ui->setBorderSize(_gui.gameInfo.windowSize.x / 600);
	ui->setMouseHoverColor(colorise(s.j("colors").j("light-gray").u("color")));
	ui->setMouseHoverColorText(colorise(s.j("colors").j("white").u("color")));
	ui->setMouseClickBorderColor(colorise(s.j("colors").j("light-white").u("color")));
	ui->setMouseClickColor(colorise(s.j("colors").j("light-gray").u("color")));
	ui->setMouseClickColorText(colorise(s.j("colors").j("white").u("color")));
	ui->setSelectedColor(colorise(s.j("colors").j("black").u("color")));
	ui->setSelectedColorText(colorise(s.j("colors").j("white").u("color")));
	_buttons.push_back(ui);
	return *ui;
}

/**
 * @brief add a button with image in the menu with menu settings
 *
 * @param pos the position
 * @param size the size
 * @param filename the path to the image
 * @param filenameHover the path to the image on hover
 * @return ButtonImageUI& a reference to the element created
 */
ButtonImageUI & AUserInterface::addButtonImage(glm::vec2 pos, glm::vec2 size,
	std::string const & filename, std::string const & filenameHover)
{
	ButtonImageUI * ui = new ButtonImageUI(pos, size, filename, filenameHover);
	_buttons.push_back(ui);
	return *ui;
}

/**
 * @brief add a slider in the menu with menu settings
 *
 * @param pos the position
 * @param size the size
 * @param min min value in slider
 * @param max max value in slider
 * @param val default value in slider
 * @param step step of the slider
 * @return SliderUI& a reference to the element created
 */
SliderUI & AUserInterface::addSlider(glm::vec2 pos, glm::vec2 size, float min, float max, float val, float step) {
	SliderUI * ui = new SliderUI(pos, size);
	ui->setValues(min, max, val, step);
	// set default color
	glm::vec4 color = colorise(s.j("colors").j("buttons").u("color"), s.j("colors").j("buttons").u("alpha"));
	ui->setColor(color);
	ui->setBorderColor(colorise(
		s.j("colors").j("buttons-border").u("color"),
		s.j("colors").j("buttons-border").u("alpha")
	));
	ui->setBorderSize(_gui.gameInfo.windowSize.x / 266);
	ui->setTextColor(colorise(
		s.j("colors").j("font").u("color"),
		s.j("colors").j("font").u("alpha")
	));

	_buttons.push_back(ui);
	return *ui;
}

/**
 * @brief add a text in the menu with menu settings
 *
 * @param pos the position
 * @param size the size
 * @param text the text
 * @return TextUI& a reference to the element created
 */
TextUI & AUserInterface::addText(glm::vec2 pos, glm::vec2 size, std::string const & text) {
	TextUI * ui = new TextUI(pos, size);
	ui->setText(text);
	if (size == VOID_SIZE)
		ui->setCalculatedSize();
	ui->setTextColor(colorise(s.j("colors").j("font").u("color")));
	_buttons.push_back(ui);
	return *ui;
}

/**
 * @brief add a Title in the menu with menu settings
 *
 * @param pos the position
 * @param size the size
 * @param text the text
 * @return TextUI& a reference to the element created
 */
TextUI & AUserInterface::addTitle(glm::vec2 pos, glm::vec2 size, std::string const & text) {
	TextUI * ui = new TextUI(pos, size);
	ui->setText(text);
	ui->setTextFont("title");
	if (size == VOID_SIZE)
		ui->setCalculatedSize();
	ui->setTextColor(colorise(s.j("colors").j("white").u("color")));
	ui->setTextOutline(0.3);
	ui->setTextOutlineColor(colorise(s.j("colors").j("black").u("color")));
	_buttons.push_back(ui);
	return *ui;
}

/**
 * @brief add a rectange in the menu with menu settings
 *
 * @param pos the position
 * @param size the size
 * @param color the rectangle color
 * @param borderColor the border rectangle color
 * @return RectUI& a reference to the element created
 */
RectUI & AUserInterface::addRect(glm::vec2 pos, glm::vec2 size, glm::vec4 color, glm::vec4 borderColor) {
	RectUI * ui = new RectUI(pos, size);
	if (color == VOID_COLOR)
		color = colorise(s.j("colors").j("bg-rect").u("color"), s.j("colors").j("bg-rect").u("alpha"));
	if (borderColor == VOID_COLOR)
		borderColor = colorise(s.j("colors").j("bg-rect-border").u("color"), s.j("colors").j("bg-rect-border").u("alpha"));
	ui->setColor(color);
	ui->setBorderColor(borderColor);
	ui->setBorderSize(_gui.gameInfo.windowSize.x / 266);
	_buttons.push_back(ui);
	return *ui;
}

/**
 * @brief add an image in the menu with menu settings
 *
 * @param pos the position
 * @param size the size
 * @param filename the path to the image
 * @return ImageUI& a reference to the element created
 */
ImageUI & AUserInterface::addImage(glm::vec2 pos, glm::vec2 size, std::string const & filename) {
	ImageUI * ui = new ImageUI(pos, size, filename);
	_buttons.push_back(ui);
	return *ui;
}

/**
 * @brief add a scrollbar in the menu with menu settings
 *
 * @param pos the position
 * @param size the size
 * @return ScrollbarUI& a reference to the element created
 */
ScrollbarUI & AUserInterface::addScrollbar(glm::vec2 pos, glm::vec2 size) {
	ScrollbarUI * ui = new ScrollbarUI(pos, size);
	glm::vec4 borderColor = colorise(
		s.j("colors").j("bg-rect-border").u("color"),
		s.j("colors").j("bg-rect-border").u("alpha")
	);
	ui->setBorderColor(borderColor);
	ui->setBorderSize(_gui.gameInfo.windowSize.x / 266);
	_buttons.push_back(ui);
	return *ui;
}

/**
 * @brief add an empty master object on the total screen in the menu with menu settings
 *
 * @return ScrollbarUI& a reference to the element created
 */
EmptyMasterUI & AUserInterface::addEmptyMaster() {
	EmptyMasterUI * ui = new EmptyMasterUI({0, 0}, {_gui.gameInfo.windowSize.x, _gui.gameInfo.windowSize.y});
	_buttons.push_back(ui);
	return *ui;
}

/**
 * @brief add a textInput in the menu with menu settings
 *
 * @param pos the position
 * @param size the size
 * @param defText the default text
 * @return TextInputUI& a reference to the element created
 */
TextInputUI & AUserInterface::addTextInput(glm::vec2 pos, glm::vec2 size, std::string const & defText) {
	TextInputUI * ui = new TextInputUI(pos, size);
	ui->setDefText(defText);
	ui->setTextColor({1, 1, 1, 1});
	ui->setTextAlign(TextAlign::LEFT);
	if (size == VOID_SIZE)
		ui->setCalculatedSize();
	_buttons.push_back(ui);
	return *ui;
}

/**
 * @brief Add an exit button on the screen
 *
 * @return ButtonImageUI& a reference to the element created
 */
ButtonImageUI & AUserInterface::addExitButton() {
	glm::vec2 winSz = _gui.gameInfo.windowSize;
	std::string filename = s.s("imgsUI") + "/cross.png";
	std::string filenameHover = s.s("imgsUI") + "/cross_hover.png";
	glm::vec2 tmpPos;
	glm::vec2 tmpSize;
	tmpSize.x = winSz.y * 0.08;
	tmpSize.y = 0;
	tmpPos.x = tmpSize.x * 0.5;
	tmpPos.y = winSz.y - tmpSize.x * 1.5;
	ButtonImageUI * ui = new ButtonImageUI(tmpPos, tmpSize, filename, filenameHover);
	ui->setKeyLeftClickInput(InputType::CANCEL);
	_buttons.push_back(ui);
	return *ui;
}

/**
 * @brief init the basic background of the menu
 *
 * @return true if success
 * @return false if error
 */
bool AUserInterface::_initBG() {
	glm::vec2 winSz = _gui.gameInfo.windowSize;
	glm::vec2 tmpPos = glm::vec2(0, 0);

	addRect(tmpPos, winSz, colorise(
		s.j("colors").j("background").u("color"),
		s.j("colors").j("background").u("alpha")
	));
	return true;
}

/* getter */
/**
 * @brief Get an UI element (button, slider, ...)
 *
 * @param id The element ID
 * @return ABaseUI& A ref to the UI
 */
ABaseUI &		AUserInterface::getUIElement(uint32_t id) { return *_buttons[id]; }
/**
 * @brief Get the total number of UI elements
 *
 * @return uint32_t The number of UI elements on the menu
 */
uint32_t		AUserInterface::getNbUIElements() const { return _buttons.size(); }

// -- Exceptions errors --------------------------------------------------------
/**
 * @brief Construct a new AUserInterface::SceneException::SceneException object
 */
AUserInterface::SceneException::SceneException()
: std::runtime_error("SceneException") {}

/**
 * @brief Construct a new AUserInterface::SceneException::SceneException object
 *
 * @param whatArg Error message
 */
AUserInterface::SceneException::SceneException(const char* whatArg)
: std::runtime_error(std::string(std::string("SceneError: ") + whatArg).c_str()) {}
