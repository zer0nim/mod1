#include <algorithm>
#include <unistd.h>

#include "Scene.hpp"

Scene::Scene(std::vector<Terrain *> & terrains)
: _terrains(terrains),
  _dtTime(0.0f),
  _fps(60),
  _wireframeMode(false) {
	_terrainId = 0;
}

Scene::~Scene() {
}

Scene::Scene(Scene const &src)
: _terrains(src._terrains) {
	*this = src;
}

Scene &Scene::operator=(Scene const &rhs) {
	if (this != &rhs) {
		logWarn("Terrain operator= called");
	}
	return *this;
}

bool	Scene::init() {
	if (!_gui.init()) {
		return false;
	}

	_gui.enableCursor(false);  // hide cursor

	return true;
}

bool	Scene::run() {
	float	maxFrameDuration = 1000 / s.j("screen").u("maxFps");
	std::chrono::milliseconds	lastLoopMs = getMs();

	while (!_gui.gameInfo.quit) {
		/* reset variables */
		_dtTime = (getMs().count() - lastLoopMs.count()) / 1000.0;
		lastLoopMs = getMs();
		_fps = 1 / _dtTime;

		if (!_update())
			return false;

		// draw
		_gui.preDraw();
		if (!_draw())
			return false;
		_gui.postDraw();

		{  // fps control
			std::chrono::milliseconds loopDuration = getMs() - lastLoopMs;
			float	frameDuration = loopDuration.count();

			if (frameDuration <= maxFrameDuration)
				usleep((maxFrameDuration - frameDuration) * 1000);
		}
	}
	return true;
}

bool	Scene::_update() {
	Inputs::update();
	_gui.update();
	_gui.cam->update(_dtTime);

	if (Inputs::getKeyByScancodeDown(SDL_SCANCODE_1))
		_wireframeMode = !_wireframeMode;

	if (Inputs::getKeyDown(InputType::ACTION)) {
		++_terrainId;
		if (_terrainId >= _terrains.size())
			_terrainId = 0;
	}

	return true;
}

bool	Scene::_draw() {
	if (_terrainId < _terrains.size() &&
		!_terrains[_terrainId]->draw(_wireframeMode)) {
			return false;
	}

	// draw skybox
	glm::mat4	view = _gui.cam->getViewMatrix();
	_gui.drawSkybox(view);
	return true;
}

// -- getters ----------------------------------------------------------
Gui &	Scene::getGui() { return _gui; }
float	Scene::getDtTime() const { return _dtTime; }

uint16_t	Scene::getFps() const {
	return std::clamp(_fps, 0.0f, float(s.j("screen").u("maxFps")));
}
