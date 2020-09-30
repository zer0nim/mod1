#ifndef WATER_HPP_
#define WATER_HPP_

#define WATER_GRID_RES glm::vec2(BOX_MAX_SIZE.x - 1, BOX_MAX_SIZE.z - 1)
#define WATER_H(u, v) (_vertices[(v) * (WATER_GRID_RES.x + 1) + (u)].pos.y)
#define WATER_MIN_DISPLAY_H 0.01

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

namespace FlowScenario {
	/**
	 * @brief Flow scenario
	 */
	enum Enum {
		EVEN_RISE = 0,
		WAVE,
		RAINING,
		DRAIN,
		SANDBOX,
		COUNT
	};
}  // namespace FlowScenario


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
		void	setScenario(uint16_t scenarioId);

		static const std::string	flowScenarioName[FlowScenario::COUNT];

	private:
		static glm::vec2 const	_gridSpace;
		static glm::vec2 const	_pipeLen;
		static float const	_gridArea;
		static std::unique_ptr<Shader>	_sh;  /**< Shader */

		Gui	& _gui;
		Terrain	& _terrain;
		bool	_firstInit;
		FlowScenario::Enum	_scenario;
		float	_gravity;  // gravity in m/s
		std::vector< std::vector<WaterColum> >	_waterCols;  // all water columns

		std::vector<WaterVert>	_vertices;
		std::vector<uint32_t>	_indices;
		uint32_t	_vao;
		uint32_t	_vbo;
		uint32_t	_ebo;

		// border mesh
		std::vector<WaterVert>	_verticesB;
		std::vector<uint32_t>	_indicesB;
		uint32_t	_vaoB;
		uint32_t	_vboB;
		uint32_t	_eboB;

		float	_currentRiseH;
		std::chrono::milliseconds	_lastRainUpdate;
		float	_maxTerrainCenterDist;

		void	_scenarioUpdate(float dtTime);
		void	_updateFlow(uint32_t u, uint32_t v, float dtTime);
		void	_updateDepth(uint32_t u, uint32_t v, float dtTime);
		void	_correctNegWaterDepth(float dtTime);
		bool	_initMesh();
		bool	_updateMesh();
		bool	_initMeshBorder();
		void	_updateBorderVertices();
		bool	_updateMeshBorder();
		float	_calculateHeight(uint32_t x, uint32_t z, float & waterDepth);
		glm::vec3	_calculateNormal(uint32_t x, uint32_t z);
		void	_staticUniform();
};

#endif  // WATER_HPP_
