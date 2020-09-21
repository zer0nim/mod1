#include <algorithm>
#include <unistd.h>

#include "Scene.hpp"

Scene::Scene(std::vector<Terrain *> & terrains)
: _terrains(terrains),
  _dtTime(0.0f),
  _fps(60),
  _wireframeMode(false),
  _orbitControls(nullptr) {
	_terrainId = 0;
	_scenarioId = 0;
	_pause = true;
}

Scene::~Scene() {
	delete _orbitControls;
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

	// init camera orbit controls
	_orbitControls = new OrbitControls(_gui);
	_orbitControls->setTarget(glm::vec3(
		BOX_MAX_SIZE.x / 2, 0, BOX_MAX_SIZE.z / 2));
	_orbitControls->setDistance(BOX_MAX_SIZE.y * 2, 30, BOX_MAX_SIZE.y * 3);

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

	// wireframe mode
	if (Inputs::getKeyByScancodeDown(SDL_SCANCODE_1))
		_wireframeMode = !_wireframeMode;

	// play pause
	if (Inputs::getKeyDown(InputType::ACTION)) {
		_pause = !_pause;
	}

	// next/previous map
	if (Inputs::getKeyDown(InputType::INCREMENT_1) ||
		Inputs::getKeyDown(InputType::DECREMENT_1))
	{
		_pause = true;
		_terrainId += Inputs::getKeyDown(InputType::INCREMENT_1) ? 1 : -1;
		if (_terrainId >= (int32_t)_terrains.size())
			_terrainId = 0;
		if (_terrainId < 0)
			_terrainId = _terrains.size() - 1;
	}

	// next scenario
	if (Inputs::getKeyDown(InputType::GOTO_MENU)) {
		_pause = true;
		++_scenarioId;
		if (_scenarioId >= FlowScenario::COUNT)
			_scenarioId = 0;
		for (Terrain * t : _terrains) {
			t->setScenario(_scenarioId);
		}
	}

	if (!_pause) {
		// update terrains/water
		if (!_terrains[_terrainId]->update(_dtTime))
			return false;
	}

	// orbit controls
	if (!_orbitControls->update(_dtTime))
		return false;
	return true;
}

bool	Scene::_draw() {
	// draw skybox
	glm::mat4	view = _gui.cam->getViewMatrix();
	_gui.drawSkybox(view);

	if (!_terrains[_terrainId]->draw(_wireframeMode)) {
			return false;
	}

	return true;
}

// -- getters ------------------------------------------------------------------
Gui &	Scene::getGui() { return _gui; }
float	Scene::getDtTime() const { return _dtTime; }

uint16_t	Scene::getFps() const {
	return std::clamp(_fps, 0.0f, float(s.j("screen").u("maxFps")));
}
