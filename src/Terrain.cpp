#include <algorithm>

#include "Terrain.hpp"

// -- Constructors -------------------------------------------------------------

Terrain::Terrain(std::string const mapPath)
: _mapPath(mapPath),
  _map(nullptr)
{
	_loadFile();
}

Terrain::~Terrain() {
	delete _map;
}

Terrain::Terrain(Terrain const &src)
: _map(nullptr) {
	*this = src;
}

Terrain &Terrain::operator=(Terrain const &rhs) {
	if (this != &rhs) {
		logWarn("Terrain operator= called");
	}
	return *this;
}

// -- Methods ------------------------------------------------------------------

void	Terrain::_loadFile() {
	_map = new SettingsJson();

	SettingsJson * coord3d = new SettingsJson();
	coord3d->add<uint64_t>("x").setMin(0).setMax(BOX_MAX_SIZE.x);
	coord3d->add<uint64_t>("y").setMin(0).setMax(BOX_MAX_SIZE.y);
	coord3d->add<uint64_t>("z").setMin(0).setMax(BOX_MAX_SIZE.z);

	_map->addList<SettingsJson>("map", coord3d);

	bool failure = false;
	try {
		if (!_map->loadFile(_mapPath)) {
			failure = true;
		}
	} catch(SettingsJson::SettingsException const & e) {
		logErr(e.what());
		failure = true;
	}

	if (failure) {
		delete _map;
		throw TerrainException(std::string("Invalid map format for: \"" +
			_mapPath + "\", see example format at \"asset/map/example1.mod1\"").c_str());
	}

	for (SettingsJson * p : _map->lj("map").list) {
		// limit points numbers to MAX_POINTS_NB
		if (_mapPoints.size() == MAX_POINTS_NB) {
			delete _map;
			throw TerrainException(std::string("Map \"" + _mapPath + "\", too many points, max number: " +
				std::to_string(MAX_POINTS_NB)).c_str());
		}

		auto eRes = _mapPoints.emplace(p->u("x"), p->u("y"), p->u("z"));
		if (!std::get<1>(eRes))
			logWarn("duplicate points in \"" << _mapPath << "\", skipped");
	}

	if (_mapPoints.size() == 0)
		throw TerrainException(std::string("Map \"" + _mapPath + "\", you need to add at least one point").c_str());

	logDebug("-- map: " << _mapPath << " ----");
	for (const glm::uvec3 & p: _mapPoints)
		logDebug(glm::to_string(p));
	logDebug("------");
}

uint32_t	Terrain::calculateHeight(glm::uvec2 pos) {
	// is pos outside the terrain limit ?
	if (pos.x > BOX_MAX_SIZE.x || pos.y > BOX_MAX_SIZE.z) {
		logErr(std::string("[calculateHeight] pos " + glm::to_string(pos) +
			" is outside the terrain limit " + glm::to_string(BOX_MAX_SIZE)).c_str());
	}

	// we already know pos height
	for (const glm::uvec3 & p: _mapPoints) {
		if (glm::uvec2(p.x, p.z) == pos)
			return p.y;
	}

	/* we need to interpolate the height */
	std::vector<HeightPoint> closPoints = _getNClosest(pos, NB_CLOSEST_POINTS);

	// inverse distance weighting
	float top = 0;
	float bottom = 0;
	for ( HeightPoint heightP : closPoints) {
		float distPow = heightP.distance;  // power of 1
		distPow *= distPow;  // power of 2, comment if you want power of 1

		top += heightP.height / distPow;
		bottom += 1 / distPow;
	}

	return top / bottom;
}

/**
 * @brief return the n closest point to pos
 *
 * @param pos the pos of the point we want to compare
 * @param n the number of points to keep
 * @return std::vector<HeightPoint> n closest points to pos
 */
std::vector<Terrain::HeightPoint>	Terrain::_getNClosest(glm::uvec2 pos, uint8_t n) {
	std::vector<HeightPoint>	allPoints;
	std::vector<HeightPoint>	res(_mapPoints.size() < n ? _mapPoints.size() : n);

	// calculate distance for each points
	for (const glm::uvec3 & p: _mapPoints) {
		HeightPoint heightP;
		heightP.distance = glm::distance(glm::vec2(pos), glm::vec2(p.x, p.z));
		heightP.height = p.y;
		allPoints.push_back(heightP);
	}

	// keep the n closest
	std::partial_sort_copy(allPoints.begin(), allPoints.end(), res.begin(), res.end(),
		[](HeightPoint lhs, HeightPoint rhs) { return lhs.distance < rhs.distance; });

	return res;
}


// -- exceptions ---------------------------------------------------------------
/**
 * @brief Construct a new Terrain::TerrainException object
 */
Terrain::TerrainException::TerrainException()
: std::runtime_error("[TerrainException]") {}

/**
 * @brief Construct a new Terrain::TerrainException object
 *
 * @param what_arg Error message
 */
Terrain::TerrainException::TerrainException(const char* what_arg)
: std::runtime_error(std::string(std::string("[TerrainException] ") +
	what_arg).c_str()) {}
