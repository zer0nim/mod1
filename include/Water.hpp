#ifndef WATER_HPP_
#define WATER_HPP_

// gravity, m/s
#define WATER_GRID_RES glm::vec2(BOX_MAX_SIZE.x - 1, BOX_MAX_SIZE.z - 1)
#define WATER_H(u, v) (_vertices[(v) * (WATER_GRID_RES.x + 1) + (u)].pos.y)

#include <vector>

#include "mod1.hpp"
#include "Terrain.hpp"

namespace FlowDir {
	/**
	 * @brief Flow directions
	 */
	enum Enum {
		LEFT,
		TOP,
		RIGHT,
		BOTTOM
	};
}  // namespace FlowDir

// flow, m3 water /s, positive flow mean increasing water level
struct	WaterColum {
	float	depth;  // water depth
	float	lFlow;  // left flow
	float	tFlow;  // top flow
	float	terrainH;  // terrain height
	WaterColum();
};

struct	WaterVert {
	glm::vec3	pos;  /**< Vert position */
	glm::vec3	norm;  /**< Vert normal */
	float		visible;  /**< Vert visibility between 0.0 and 1.0 */
};

class Water {
	public:
		Water(Terrain & terrain, Gui & gui);
		virtual ~Water();
		Water(Water const &src);
		Water &operator=(Water const &rhs);

		bool	init();
		bool	update(float dtTime);
		bool	draw(bool wireframe = false);

	private:
		static glm::vec2 const	_gridSpace;
		static glm::vec2 const	_pipeLen;
		static float const	_gridArea;
		static std::unique_ptr<Shader>	_sh;  /**< Shader */

		Gui	& _gui;
		Terrain	& _terrain;
		float	_gravity;  // gravity in m/s
		std::vector< std::vector<WaterColum> >	_waterCols;  // all water columns

		std::vector<WaterVert>	_vertices;
		std::vector<uint32_t>	_indices;
		uint32_t	_vao;
		uint32_t	_vbo;
		uint32_t	_ebo;

		void	_updateFlow(uint32_t u, uint32_t v, float dtTime);
		void	_updateDepth(uint32_t u, uint32_t v, float dtTime);
		void	_correctNegWaterDepth(float dtTime);
		bool	_initMesh();
		bool	_updateMesh();
		float	_calculateHeight(uint32_t x, uint32_t z, bool & noWater);
		glm::vec3	_calculateNormal(uint32_t x, uint32_t z);
		void	_staticUniform();
};

#endif  // WATER_HPP_
