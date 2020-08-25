#include <algorithm>

#include "Terrain.hpp"

// -- Constructors -------------------------------------------------------------

Terrain::Terrain(std::string const mapPath, Gui & gui)
: _gui(gui),
  _mapPath(mapPath),
  _map(nullptr),
  _vao(0),
  _vbo(0),
  _ebo(0)
{
	// init static shader if null
	if (!_sh) {
		_sh = std::unique_ptr<Shader>(
			new Shader("shaders/terrain_vs.glsl", "shaders/terrain_fs.glsl"));
	}

	_loadFile();
}

Terrain::~Terrain() {
	delete _map;

	// free vao / vbo
	_sh->use();
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &_vbo);
	glDeleteBuffers(1, &_ebo);
	glDeleteVertexArrays(1, &_vao);
	_sh->unuse();
}

Terrain::Terrain(Terrain const &src)
: _gui(src._gui),
  _map(nullptr),
  _vao(0),
  _vbo(0),
  _ebo(0) {
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
	std::string errMsg;
	try {
		if (!_map->loadFile(_mapPath)) {
			failure = true;
		}
	} catch(SettingsJson::SettingsException const & e) {
		errMsg = e.what();
		failure = true;
	}

	if (failure) {
		delete _map;
		throw TerrainException(std::string(errMsg +
			", compare with the example: \"asset/map/example1.mod1\"").c_str());
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
}

bool	Terrain::draw(bool wireframe) {
	_sh->use();

	// update uniforms
	_sh->setMat4("view", _gui.cam->getViewMatrix());
	_sh->setVec3("viewPos", _gui.cam->pos);

	glBindVertexArray(_vao);
	if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLE_STRIP, _indices.size(), GL_UNSIGNED_INT, 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // reset polygon mode
	glBindVertexArray(0);


	_sh->unuse();

	return true;
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

glm::vec3	Terrain::_calculateNormal(uint32_t x, uint32_t z) {
	float hL, hR, hB, hT;

	x = x == 0 ? 1 : x;
	z = z == 0 ? 1 : z;

	// hL
	hL = TERRAIN_H(x - 1, z);
	// hR
	if (x < BOX_MAX_SIZE.x - 1)
		hR = TERRAIN_H(x + 1, z);
	else
		hR = TERRAIN_H(x, z);
	// hT
	if (z < BOX_MAX_SIZE.z - 1)
		hT = TERRAIN_H(x, z + 1);
	else
		hT = TERRAIN_H(x, z);
	// hB
	hB = TERRAIN_H(x, z - 1);

	float sx = hR - hL;
	if (x == 0 || x == BOX_MAX_SIZE.x - 1)
		sx *= 2;

	float sy = hB - hT;
	if (z == 0 || z == BOX_MAX_SIZE.z -1)
		sy *= 2;

	glm::vec3 norm(-sx, 2.0, sy);

	return glm::normalize(norm);
}

bool	Terrain::initMesh() {
	// fill vertices
	for (uint16_t z = 0; z < BOX_MAX_SIZE.z; ++z) {
		for (uint16_t x = 0; x < BOX_MAX_SIZE.x; ++x) {
			TerrainVert	vert;
			vert.pos = {x, calculateHeight({x, z}), z};
			_vertices.push_back(vert);
		}
	}

	// fill vert colors
	_initColors();

	// calc vertices normals
	for (TerrainVert & vert : _vertices) {
		vert.norm = _calculateNormal(vert.pos.x, vert.pos.z);
		vert.norm *= DISPLAY_RES;
	}

	// fill indices
	// By repeating the last vertex and the first vertex, we created four
	// degenerate triangles that will be skipped, and linked the first row
	// with the second. We could link an arbitrary number of rows this way and
	// draw the entire mesh with only one call
	// cf: learnopengles.com/tag/triangle-strips
	for (uint16_t y = 0; y < BOX_MAX_SIZE.z - 1; ++y) {
		// duplicate first vertice to generate degenerate triangle
		if (y > 0)
			_indices.push_back(y * BOX_MAX_SIZE.x);

		uint32_t a = 0;
		uint32_t b = 0;
		for (uint16_t x = 0; x < BOX_MAX_SIZE.x; ++x) {
			a = x + y * BOX_MAX_SIZE.x;
			b = a + BOX_MAX_SIZE.x;
			_indices.push_back(a);
			_indices.push_back(b);
		}

		// duplicate last vertice to generate degenerate triangle
		if (y != BOX_MAX_SIZE.z - 2)
			_indices.push_back(b);
	}

	// create vao, vbo, ebo
	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);
	glGenBuffers(1, &_ebo);

	// fill vao buffer
	glBindVertexArray(_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(TerrainVert),
		&_vertices[0], GL_STATIC_DRAW);

	// set-up ebo
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(u_int32_t),
		&_indices[0], GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVert),
		reinterpret_cast<void *>(0));
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVert),
		reinterpret_cast<void *>(offsetof(TerrainVert, norm)));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVert),
		reinterpret_cast<void *>(offsetof(TerrainVert, color)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	_staticUniform();

	return true;
}

void	Terrain::_initColors() {
	std::array<glm::vec3, 3>	colors = {
		glm::vec3(0.92f, 0.85f, 0.69f),  // rgb(235, 217, 177)
		glm::vec3(0.61f, 0.76f, 0.2f),  // rgb(156, 195, 53)
		glm::vec3(0.54f, 0.5f, 0.56f)  // rgb(136, 130, 144)
	};

	// calc min/max height
	float minH = _vertices[0].pos.y;
	float maxH = minH;
	for (TerrainVert & vert : _vertices) {
		if (vert.pos.y < minH)
			minH = vert.pos.y;
		if (vert.pos.y > maxH)
			maxH = vert.pos.y;
	}

	float diffH = maxH - minH;
	float step = 1.0 / (colors.size() - 1);
	for (TerrainVert & vert : _vertices) {
		float ratio = (vert.pos.y - minH) / diffH;

		for (uint8_t i = 0; i < colors.size() - 1; ++i) {
			if (ratio <= step * (i + 1)) {
				ratio /= step * (i + 1);
				vert.color = glm::lerp(colors[i], colors[i+1], ratio);
				break;
			}
		}
	}
}


void	Terrain::_staticUniform() {
	_sh->use();

	// camera projection
	_sh->setMat4("projection", _gui.cam->getProjection());

	// direction light
	_sh->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
	_sh->setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	_sh->setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
	_sh->setVec3("dirLight.specular", 0.3f, 0.3f, 0.3f);

	// terrain material
	Material material;
	material.diffuse = {0.5f, 0.5f, 0.5f};
	material.shininess = 32.0f;
	// specular
	_sh->setVec3("material.specular", material.specular);
	// shininess
	_sh->setFloat("material.shininess", material.shininess);

	_sh->unuse();
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

// -- static initialisation ----------------------------------------------------
std::unique_ptr<Shader> Terrain::_sh = nullptr;
