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
			_mapPath + '"').c_str());
	}

	for (SettingsJson * test : _map->lj("map").list) {
		logDebug("x: " << test->u("x") <<
			", y: " << test->u("y") <<
			", z: " << test->u("z"));
	}
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
