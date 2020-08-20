#include <algorithm>
#include <unistd.h>

#include "Scene.hpp"

Scene::Scene()
: _dtTime(0.0f),
  _fps(60) {
}

Scene::~Scene() {
}

Scene::Scene(Scene const &src) {
	*this = src;
}

Scene &Scene::operator=(Scene const &rhs) {
	if (this != &rhs) {}
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
	return true;
}

bool	Scene::_draw() {
	// draw skybox
	glm::mat4	view = _gui.cam->getViewMatrix();
	_gui.drawSkybox(view);
	return true;
}

// -- getters ----------------------------------------------------------
float	Scene::getDtTime() const { return _dtTime; }

uint16_t	Scene::getFps() const {
	return std::clamp(_fps, 0.0f, float(s.j("screen").u("maxFps")));
}
