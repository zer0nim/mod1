#include "MouseRaycast.hpp"

namespace MouseRaycast {
	glm::vec3	calcMouseRay(Gui & gui) {
		glm::ivec2 mouse = Inputs::getMousePos();

		// 3d Normalised Device Coordinates
		glm::vec2 rayNDC;
		rayNDC.x = (mouse.x * 2.0f) / gui.gameInfo.windowSize.x - 1.0f;
		rayNDC.y = 1.0f - (mouse.y * 2.0f) / gui.gameInfo.windowSize.y;

		// 4d Homogeneous Clip Coordinates
		glm::vec4 rayClip = glm::vec4(rayNDC.x, rayNDC.y, -1.0, 1.0);

		// 4d Eye (Camera) Coordinates
		glm::vec4 rayEye = glm::inverse(gui.cam->getProjection()) * rayClip;
		rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);

		// 3d World Coordinates
		glm::vec3 rayWorld = glm::inverse(gui.cam->getViewMatrix()) * rayEye;
		glm::normalize(rayWorld);

		return rayWorld;
	}

	// -- terrain raycast ------------------------------------------------------

	glm::vec3	getPointOnRay(glm::vec3 camPos, glm::vec3 ray, float distance) {
		return camPos + ray * distance;
	}

	bool	isUnderGround(Terrain & terrain, glm::vec3 pos) {
		float height = 0.0f;
		terrain.getNearHeight(pos.x, pos.z, height);
		return pos.y < height;
	}

	bool	intersectionInRange(Terrain & terrain, glm::vec3 camPos, float start, float finish, glm::vec3 ray) {
		glm::vec3 startPoint = getPointOnRay(camPos, ray, start);
		glm::vec3 endPoint = getPointOnRay(camPos, ray, finish);
		return !isUnderGround(terrain, startPoint) && isUnderGround(terrain, endPoint);
	}


	glm::vec3	binarySearch(Terrain & terrain, glm::vec3 camPos, int count, float start, float finish, glm::vec3 ray) {
		float half = start + ((finish - start) / 2.0f);
		if (count >= BINARY_SEARCH_ITERATIONS) {
			glm::vec3 endPoint = getPointOnRay(camPos, ray, half);
			return endPoint;
		}

		if (intersectionInRange(terrain, camPos, start, half, ray)) {
			return binarySearch(terrain, camPos, count + 1, start, half, ray);
		}
		return binarySearch(terrain, camPos, count + 1, half, finish, ray);
	}

	bool	updateTerrainPos(Terrain & terrain, glm::vec3 camPos, glm::vec3 & ray,
		float len, glm::vec3 & intersection)
	{
		intersection = binarySearch(terrain, camPos, 0, 0, len, ray);
		intersection.x = std::round(intersection.x);
		intersection.z = std::round(intersection.z);
		return (intersection.x >= 0 && intersection.x < BOX_MAX_SIZE.x &&
			intersection.z >= 0 && intersection.z < BOX_MAX_SIZE.z);
	}
}
