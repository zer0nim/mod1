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
	coord3d->add<uint64_t>("x").setMin(0).setMax(100);
	coord3d->add<uint64_t>("y").setMin(0).setMax(100);
	coord3d->add<uint64_t>("z").setMin(0).setMax(100);

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
		throw TerrainException(std::string("Invalid map format for: \"" +
			_mapPath + "\", see example format at 'asset/map/example1.mod1'").c_str());
	}

	for (SettingsJson * p : _map->lj("map").list) {
		auto eRes = _mapPoints.emplace(p->u("x"), p->u("y"), p->u("z"));
		if (!std::get<1>(eRes))
			logWarn("duplicate points in \"" << _mapPath << "\", skipped");
	}

	for (const glm::uvec3 & p: _mapPoints)
		logDebug(glm::to_string(p));
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
