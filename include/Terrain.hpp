#ifndef TERRAIN_HPP_
#define TERRAIN_HPP_

#define NB_CLOSEST_POINTS 8
#define DISPLAY_RES 0.5
#define TERRAIN_H(u, v) (_vertices[(v) * BOX_MAX_SIZE.x + (u)].pos.y)
#define TESR_H(u, v) (_vertices[(v) * BOX_MAX_SIZE.x + (u)].pos)

#include <string>
#include <unordered_set>
#include <vector>

#include "mod1.hpp"
#include "Shader.hpp"
#include "Gui.hpp"
#include "Material.hpp"

struct	TerrainVert {
	glm::vec3	pos;  /**< Vert position */
	glm::vec3	norm;  /**< Vert normal */
	glm::vec3	color;  /**< Vert color */
};

/**
 * @brief class to manage terrain creation
 * It use the inverse distance weighting algorithm to interpolate height values
 * between points described at:
 * gisgeography.com/inverse-distance-weighting-idw-interpolation
 */
class Terrain {
	public:
		Terrain(std::string mapPath, Gui & gui);
		virtual ~Terrain();
		Terrain(Terrain const &src);
		Terrain &operator=(Terrain const &rhs);

		bool		draw(bool wireframe = false);
		uint32_t	calculateHeight(glm::uvec2 pos);
		bool		initMesh();

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
		glm::vec3	_calculateNormal(uint32_t x, uint32_t z);
		void	_initColors();
		void	_staticUniform();

		static std::unique_ptr<Shader>	_sh;  /**< Shader */
		Gui				&_gui;
		std::string		_mapPath;
		SettingsJson	*_map;
		std::unordered_set<glm::uvec3>	_mapPoints;

		std::vector<TerrainVert>	_vertices;
		std::vector<uint32_t>	_indices;
		uint32_t	_vao;
		uint32_t	_vbo;
		uint32_t	_ebo;
};

#endif  // TERRAIN_HPP_
