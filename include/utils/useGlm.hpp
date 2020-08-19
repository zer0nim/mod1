#ifndef USEGLM_HPP_
#define USEGLM_HPP_

// glm
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>  // to print vect/mat with glm::to_string
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/hash.hpp>

/**
 * @brief to use glm::vec as map key and add lerp function to glm
 */
namespace glm {
	/**
	 * @brief Lerp (Linear interpolation) on vector
	 *
	 * @tparam T The vector type
	 * @param start The start vector
	 * @param end The end vector
	 * @param factor The factor (btw 0 & 1)
	 * @return tvec3<T> The vector after lerp
	 */
	template <typename T>
	tvec3<T>	lerp(tvec3<T> start, tvec3<T> end, float factor) {
		return (start + factor * (end - start));
	}

	/**
	 * @brief Lerp (Linear interpolation) on vector
	 *
	 * @tparam T The vector type
	 * @param start The start vector
	 * @param end The end vector
	 * @param factor The factor (btw 0 & 1)
	 * @return tvec3<T> The vector after lerp
	 */
	template <typename T>
	tvec2<T>	lerp(tvec2<T> start, tvec2<T> end, float factor) {
		return (start + factor * (end - start));
	}
};  // namespace glm

#endif  // USEGLM_HPP_
