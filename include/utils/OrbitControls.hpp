#ifndef ORBITCONTROLS_HPP_
#define ORBITCONTROLS_HPP_

#include "Gui.hpp"
#include "Logging.hpp"
#include "useGlm.hpp"
#include "Camera.hpp"
#include "ETransform.hpp"

class OrbitControls {
	public:
		/**
		 * @brief Construct a new Orbit Controls object
		 *
		 * @param gui
		 * @param startVertAngle set the start vertical angle between:
		 *  -90° (bellow) and 90° (above), 0° is just in front
		 */
		OrbitControls(Gui & gui, float startVertAngle = 45);
		virtual ~OrbitControls();
		OrbitControls(OrbitControls const &src);
		OrbitControls &operator=(OrbitControls const &rhs);

		bool	update(float dtTime);
		void	setDistance(int32_t dist, int32_t min, int32_t max);
		void	setTarget(glm::vec3 target);
		float	getDistance() const;

	private:
		glm::vec3	_target;
		float	_distance;
		float	_minDistance;
		float	_maxDistance;
		Gui		& _gui;
		Camera	& _cam;
		ETransform	transform;
		bool	_mousePressed;
		float	_verticalAngle;
};

#endif  // ORBITCONTROLS_HPP_
