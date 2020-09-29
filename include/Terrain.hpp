#ifndef TERRAIN_HPP_
#define TERRAIN_HPP_

#define NB_CLOSEST_POINTS 16
#define BOX_B_STEP 8
#define TERRAIN_H(u, v) (_vertices[(v) * BOX_MAX_SIZE.x + (u)].pos.y)

#include <string>
#include <unordered_set>
#include <vector>

#include "mod1.hpp"
#include "Shader.hpp"
#include "Gui.hpp"
#include "Material.hpp"

class Scene;
class Water;

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
		Terrain(std::string mapPath, Gui & gui, Scene & scene);
		virtual ~Terrain();
		Terrain(Terrain const &src);
		Terrain &operator=(Terrain const &rhs);

		bool	init();
		bool	update(float dtTime);
		bool	draw(bool wireframe = false);
		void	setScenario(uint16_t scenarioId);
		float	getHeight(uint32_t u, uint32_t v) const;
		bool	getNearHeight(float u, float v, float & height) const;
		float	getMinHeight() const;
		float	getMaxHeight() const;
		float	getOrbitDistance() const;

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
			int16_t distance;
			int16_t	height;
		};

		void	_loadFile();
		bool	_initMesh();
		bool	_initMeshBorder();
		std::vector<HeightPoint>	_getNClosest(glm::uvec2 pos, uint8_t n);
		float	_calculateHeight(glm::uvec2 pos);
		glm::vec3	_calculateNormal(uint32_t x, uint32_t z);
		void	_initColors();
		glm::vec3	_calcColor(float ratio);
		void	_staticUniform();

		static std::unique_ptr<Shader>	_sh;  /**< Shader */
		static std::array<glm::vec3, 3>	_colors;
		Gui				&_gui;
		Scene			&_scene;
		std::string		_mapPath;
		SettingsJson	*_map;
		std::unordered_set<glm::vec3>	_mapPoints;

		std::vector<TerrainVert>	_vertices;
		std::vector<uint32_t>	_indices;
		uint32_t	_vao;
		uint32_t	_vbo;
		uint32_t	_ebo;

		// border mesh
		std::vector<TerrainVert>	_verticesB;
		std::vector<uint32_t>	_indicesB;
		uint32_t	_vaoB;
		uint32_t	_vboB;
		uint32_t	_eboB;
		glm::vec3	_borderColor;

		Water	*_water;
		float	_minH;
		float	_maxH;
};

#endif  // TERRAIN_HPP_
