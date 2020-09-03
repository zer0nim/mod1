#ifndef WATER_HPP_
#define WATER_HPP_

// gravity, m/s
#define WATER_GRID_RES glm::vec2(8, 8)

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

class Water {
	public:
		Water(Terrain & terrain);
		virtual ~Water();
		Water(Water const &src);
		Water &operator=(Water const &rhs);

		void	init();
		bool	update(float dtTime);
		bool	draw();

	private:
		static glm::vec2 const	_gridSpace;
		static glm::vec2 const	_pipeLen;
		static float const	_gridArea;

		Terrain & _terrain;
		float	_gravity;  // gravity in m/s
		std::vector< std::vector<WaterColum> >	_waterCols;  // all water columns

		void	_updateFlow(uint32_t u, uint32_t v, float dtTime);
		void	_updateDepth(uint32_t u, uint32_t v, float dtTime);
		void	_correctNegWaterDepth(float dtTime);
};

#endif  // WATER_HPP_
