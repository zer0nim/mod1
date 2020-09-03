#include "Water.hpp"

// -- const --------------------------------------------------------------------
// space between grid points
glm::vec2 const	Water::_gridSpace = glm::vec2(
	BOX_MAX_SIZE.x / WATER_GRID_RES.x, BOX_MAX_SIZE.z / WATER_GRID_RES.y);

// water grid pipe length
glm::vec2 const	Water::_pipeLen = _gridSpace;

// grid box area
float const	Water::_gridArea = Water::_gridSpace.x * Water::_gridSpace.y;

// -- members ------------------------------------------------------------------
Water::Water(Terrain & terrain)
: _terrain(terrain) {
	_gravity = 9.81;
	// allocate _waterCols 2d array
	_waterCols = std::vector< std::vector<WaterColum> >(
		WATER_GRID_RES.y, std::vector<WaterColum>(WATER_GRID_RES.x, WaterColum()));
}

Water::~Water() {
}

Water::Water(Water const &src)
: _terrain(src._terrain){
	*this = src;
}

Water &Water::operator=(Water const &rhs) {
	if (this != &rhs) {}
	return *this;
}

void	Water::init() {
	// init column terrain height now to avoid retrieving it every update
	for (uint32_t v = 0; v < WATER_GRID_RES.y; ++v) {
		for (uint32_t u = 0; u < WATER_GRID_RES.x; ++u) {
			uint32_t tU = (uint32_t)((u + 0.5) * _gridSpace.x);
			uint32_t tV = (uint32_t)((v + 0.5) * _gridSpace.y);
			_waterCols[v][u].terrainH = _terrain.getHeight(tU, tV);

			// init left waters column at 0 to test
			if (u == 0)
				_waterCols[v][u].depth = 50.0;
		}
	}
}

bool	Water::update(float dtTime) {
	// update all water columns
	for (uint32_t v = 0; v < WATER_GRID_RES.y; ++v) {
		for (uint32_t u = 0; u < WATER_GRID_RES.x; ++u) {
			_updateFlow(u, v, dtTime);
			_updateDepth(u, v, dtTime);
		}
	}

	_correctNegWaterDepth(dtTime);
	return true;
}

void	Water::_updateFlow(uint32_t u, uint32_t v, float dtTime) {
	/*
		pipeCSA is the cross-sectional area of the pipe, what value do I need ? 🤔
		We calculate pipeCSA = _d * ∆x, where _d is the upwind depth as in equation 4.5.
		_d is the depth evaluated at the upwind direction:

		// is u the flow ?
		if ((u(i + 1/2, j) > 0) {
			_d(i + 1/2, j) = d(i, j);
		}
		else {
			_d(i + 1/2, j) = d(i + 1, j);
		}

		Artificially varying pipeCSA leads to an approximate method for modeling viscosity (larger values make the water more lively).
	*/
	float pipeCSA = 1.0;  // cross-sectional area of the pipe

	// column water total height
	float totalH = _waterCols[v][u].terrainH + _waterCols[v][u].depth;

	/* left flow */
	// if there is a wall, set flow to 0
	if (u == 0 || (_waterCols[v][u - 1].depth == 0 && _waterCols[v][u - 1].terrainH > totalH)) {
		_waterCols[v][u].lFlow = 0.0;
	}
	else {
		float totalLeftH = _waterCols[v][u - 1].terrainH + _waterCols[v][u - 1].depth;
		float hDiff = totalLeftH - totalH;
		_waterCols[v][u].lFlow += pipeCSA * (_gravity / _pipeLen.x) * hDiff * dtTime;
	}

	/* top flow */
	// if there is a wall, set flow to 0
	if (v == 0 || (_waterCols[v - 1][u].depth == 0 && _waterCols[v - 1][u].terrainH > totalH)) {
		_waterCols[v][u].tFlow = 0.0;
	}
	else {
		float totalTopH = _waterCols[v - 1][u].terrainH + _waterCols[v - 1][u].depth;
		float hDiff = totalTopH - totalH;
		_waterCols[v][u].tFlow += pipeCSA * (_gravity / _pipeLen.y) * hDiff * dtTime;
	}

	// right and bottom flow will be processed by right and bottom column update
}

void	Water::_updateDepth(uint32_t u, uint32_t v, float dtTime) {
	float totalFlow = 0.0;  // we store the total amount of flow here
	// left flow
	totalFlow += _waterCols[v][u].lFlow;
	// top flow
	totalFlow += _waterCols[v][u].tFlow;
	// right flow
	if (u < WATER_GRID_RES.x - 1)
		totalFlow += -_waterCols[v][u + 1].lFlow;
	// bottom flow
	if (v < WATER_GRID_RES.y - 1)
		totalFlow += -_waterCols[v + 1][u].tFlow;

	// calculate the new depth
	_waterCols[v][u].depth += (totalFlow / _gridArea) * dtTime;
}

void	Water::_correctNegWaterDepth(float dtTime) {
	// search for negativ depth and correct them
	for (uint32_t v = 0; v < WATER_GRID_RES.y; ++v) {
		for (uint32_t u = 0; u < WATER_GRID_RES.x; ++u) {
			// depth should not be negative
			if (_waterCols[v][u].depth < 0.0) {
				float dDiff = -_waterCols[v][u].depth;
				float totalNegFlow = 0.0;

				// calculate the total negative flow, to calculate ratio later
				std::vector<FlowDir::Enum>	negativeFlows;
				if (_waterCols[v][u].lFlow < 0.0) {
					negativeFlows.push_back(FlowDir::LEFT);
					totalNegFlow += -_waterCols[v][u].lFlow;
				}
				if (_waterCols[v][u].tFlow < 0.0) {
					negativeFlows.push_back(FlowDir::TOP);
					totalNegFlow += -_waterCols[v][u].tFlow;
				}
				if (u < WATER_GRID_RES.x - 1 && _waterCols[v][u + 1].lFlow > 0.0) {
					negativeFlows.push_back(FlowDir::RIGHT);
					totalNegFlow += _waterCols[v][u + 1].lFlow;
				}
				if (v < WATER_GRID_RES.y - 1 && _waterCols[v + 1][u].tFlow > 0.0) {
					negativeFlows.push_back(FlowDir::BOTTOM);
					totalNegFlow += _waterCols[v + 1][u].tFlow;
				}

				// update flow and neighbor height proportionally
				float correcRatio = 0.0;
				float correcFlow = 0.0;
				for (FlowDir::Enum flowDir : negativeFlows) {
					switch (flowDir) {
					case FlowDir::LEFT:
						correcRatio = -_waterCols[v][u].lFlow / totalNegFlow;
						correcFlow = dDiff * correcRatio;
						_waterCols[v][u].lFlow += correcFlow;
						_waterCols[v][u - 1].depth -= (correcFlow / _gridArea) * dtTime;
						break;
					case FlowDir::TOP:
						correcRatio = -_waterCols[v][u].tFlow / totalNegFlow;
						correcFlow = dDiff * correcRatio;
						_waterCols[v][u].tFlow += correcFlow;
						_waterCols[v - 1][u].depth -= (correcFlow / _gridArea) * dtTime;
						break;
					case FlowDir::RIGHT:
						correcRatio = _waterCols[v][u + 1].lFlow / totalNegFlow;
						correcFlow = dDiff * correcRatio;
						_waterCols[v][u + 1].lFlow -= correcFlow;
						_waterCols[v][u + 1].depth -= (correcFlow / _gridArea) * dtTime;
						break;
					case FlowDir::BOTTOM:
						correcRatio = _waterCols[v + 1][u].tFlow / totalNegFlow;
						correcFlow = dDiff * correcRatio;
						_waterCols[v + 1][u].tFlow -= correcFlow;
						_waterCols[v + 1][u].depth -= (correcFlow / _gridArea) * dtTime;
						break;
					}
				}

				// set negative water depth to 0
				_waterCols[v][u].depth = 0.0;
			}
		}
	}
}

bool	Water::draw() {
	logDebug("- draw ---------------------");
	for (uint32_t v = 0; v < WATER_GRID_RES.y; ++v) {
		std::string line;
		for (uint32_t u = 0; u < WATER_GRID_RES.x; ++u) {
			line += std::to_string(_waterCols[v][u].depth) + ", ";
		}
		logDebug(line);
	}
	return true;
}

// -- WaterColum ---------------------------------------------------------------
WaterColum::WaterColum() {
	depth = 0.0;
	lFlow = 0.0;
	tFlow = 0.0;
	terrainH = 0.0;
}
