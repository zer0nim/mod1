#ifndef TERRAIN_HPP_
#define TERRAIN_HPP_

#define NB_CLOSEST_POINTS 3

#include <string>
#include <unordered_set>
#include <vector>

#include "mod1.hpp"

/**
 * @brief class to manage terrain creation
 * It use the inverse distance weighting algorithm to interpolate height values
 * between points described at:
 * gisgeography.com/inverse-distance-weighting-idw-interpolation
 */
class Terrain {
	public:
		Terrain(std::string mapPath);
		virtual ~Terrain();
		Terrain(Terrain const &src);
		Terrain &operator=(Terrain const &rhs);

		uint32_t	calculateHeight(glm::uvec2 pos);

		// -- exceptions -------------------------------------------------------
		/**
		 * @brief Terrain exception
		 */
		class TerrainException : public std::runtime_error {
			public:
				TerrainException();
				explicit TerrainException(const char* what_arg);
		};

	private:
		/**
		 * @brief Info about a heightmap point
		 */
		struct HeightPoint {
			uint16_t	distance;
			uint16_t	height;
		};

		void	_loadFile();
		std::vector<HeightPoint>	_getNClosest(glm::uvec2 pos, uint8_t n);

		std::string		_mapPath;
		SettingsJson	*_map;
		std::unordered_set<glm::uvec3>	_mapPoints;
};

#endif  // TERRAIN_HPP_
