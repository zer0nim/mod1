#ifndef MOUSERAYCAST_HPP_
#define MOUSERAYCAST_HPP_

#define BINARY_SEARCH_ITERATIONS 100

#include "mod1.hpp"
#include "Inputs.hpp"
#include "Gui.hpp"
#include "Terrain.hpp"

namespace MouseRaycast {
	glm::vec3	calcMouseRay(Gui & gui);

	bool	updateTerrainPos(Terrain & terrain, glm::vec3 camPos, glm::vec3 & ray,
		float len, glm::vec3 & intersection);

}  // namespace MouseRaycast

#endif  // MOUSERAYCAST_HPP_
