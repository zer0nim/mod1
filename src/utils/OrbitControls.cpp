#include "OrbitControls.hpp"

OrbitControls::OrbitControls(Gui & gui, float startVertAngle)
: _gui(gui),
  _cam(*_gui.cam) {
	_distance = 0;
	_minDistance = 0;
	_maxDistance = 0;
	setTarget({0, 0, 0});
	setDistance(10, 1, 20);
	_mousePressed = false;
	_verticalAngle = startVertAngle;
}

OrbitControls::~OrbitControls() {
}

OrbitControls::OrbitControls(OrbitControls const &src)
: _gui(src._gui),
  _cam(src._cam) {
	*this = src;
}

OrbitControls &OrbitControls::operator=(OrbitControls const &rhs) {
	if (this != &rhs) {
		logWarn("OrbitControls operator= called");
	}
	return *this;
}

// -- members ------------------------------------------------------------------

bool	OrbitControls::update(float dtTime) {
	if (_gui.getWindowsFlag(SDL_WINDOW_INPUT_FOCUS)) {
		if (Inputs::getLeftClickDown())
			_mousePressed = true;
		if (Inputs::getLeftClickUp())
			_mousePressed = false;

		if (_mousePressed) {
			// mouse movement
			glm::vec2 offset = Inputs::getMouseRel();
			offset *= _cam.mouseSensitivity;

			float speed = 200;
			transform.setPos(_cam.pos);

			// horizontal rotation
			float angle = -offset.x * speed * dtTime;
			transform.rotateAround(_target, {0, 1, 0}, angle);
			_cam.pos = transform.getPos();

			// vertical rotation
			angle = -offset.y * speed * dtTime;
			float newVertAngle = _verticalAngle - angle;
			if (newVertAngle > 89) {
				angle = -(89.0f - _verticalAngle);
			}
			else if (newVertAngle < 0) {
				angle = _verticalAngle;
			}
			_verticalAngle -= angle;

			if (angle != 0) {
				transform.rotateAround(_target, _cam.right, angle);
				_cam.pos = transform.getPos();
			}

			// look at target
			_cam.lookAt(_target);
		}

		// distance (mouse wheel)
		glm::ivec2 scroll = Inputs::getMouseScroll();
		if (scroll != glm::ivec2(0, 0)) {
			float scrollSpeed = 250;
			_distance -= scroll.y * scrollSpeed * dtTime;
			_distance = std::max(_minDistance, _distance);
			_distance = std::min(_maxDistance, _distance);
			setDistance(_distance, _minDistance, _maxDistance);
		}
	}

	return true;
}

void	OrbitControls::setDistance(int32_t dist, int32_t min, int32_t max) {
	if (min > dist || max < dist || min > max) {
		logErr("setDistance(" << dist << ", " << min << ", " << max << "), "
			" impossible args: min <= dist <= max");
		return;
	}
	else {
		_distance = dist;
		_minDistance = min;
		_maxDistance = max;
	}

	// move the camera to match the distance
	float crntDist = glm::distance(_cam.pos, _target);
	if (crntDist != 0) {
		glm::vec3 dir = glm::normalize( _cam.pos - _target);
		float diff = _distance - crntDist;
		_cam.pos += dir * diff;
	}
	else {
		_cam.pos = _target;
		glm::vec3 dir = glm::rotateX(glm::vec3(0, 0, 1), glm::radians(-_verticalAngle));

		_cam.pos += _distance * dir;
	}

	// corect orientation after movement
	_cam.lookAt(_target);
}

void	OrbitControls::setTarget(glm::vec3 target) {
	_target = target;
	_cam.pos = _target;
	setDistance(_distance, _minDistance, _maxDistance);
	_cam.lookAt(_target);
}

// -- getters ------------------------------------------------------------------
float	OrbitControls::getDistance() const { return _distance; }
