#include <algorithm>

#include "Terrain.hpp"
#include "Scene.hpp"
#include "Water.hpp"

// -- Constructors -------------------------------------------------------------

Terrain::Terrain(std::string const mapPath, Gui & gui, Scene & scene)
: _gui(gui),
  _scene(scene),
  _mapPath(mapPath),
  _map(nullptr),
  _vao(0),
  _vbo(0),
  _ebo(0),
  _vaoB(0),
  _vboB(0),
  _eboB(0),
  _minH(0),
  _maxH(0)
{
	// init static shader if null
	if (!_sh) {
		_sh = std::unique_ptr<Shader>(
			new Shader("shaders/terrain_vs.glsl", "shaders/terrain_fs.glsl"));
	}

	_loadFile();

	_water = new Water(*this, _gui);
}

Terrain::~Terrain() {
	// free vao / vbo
	_sh->use();
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &_vbo);
	glDeleteBuffers(1, &_ebo);
	glDeleteVertexArrays(1, &_vao);
	glDeleteBuffers(1, &_vboB);
	glDeleteBuffers(1, &_eboB);
	glDeleteVertexArrays(1, &_vaoB);
	_sh->unuse();

	delete _water;
}

Terrain::Terrain(Terrain const &src)
: _gui(src._gui),
  _scene(src._scene),
  _map(nullptr),
  _vao(0),
  _vbo(0),
  _ebo(0),
  _vaoB(0),
  _vboB(0),
  _eboB(0),
  _minH(0),
  _maxH(0) {
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
	coord3d->add<int64_t>("x").setMin(1).setMax(BOX_MAX_SIZE.x - 1);
	coord3d->add<int64_t>("y").setMin(-BOX_GROUND_HEIGHT).setMax(BOX_MAX_SIZE.y - BOX_GROUND_HEIGHT);
	coord3d->add<int64_t>("z").setMin(1).setMax(BOX_MAX_SIZE.z - 1);

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

		auto eRes = _mapPoints.emplace(p->i("x"), p->i("y"), p->i("z"));
		if (!std::get<1>(eRes))
			logWarn("duplicate points in \"" << _mapPath << "\", skipped");
	}

	delete _map;

	// fill map border with 0 altitude
	for (uint16_t x = 0; x < BOX_MAX_SIZE.x; x += BOX_B_STEP) {
		_mapPoints.emplace(x, 0, 0);
		_mapPoints.emplace(x, 0, BOX_MAX_SIZE.z - 1);
	}
	for (uint16_t z = 0; z < BOX_MAX_SIZE.z; z += BOX_B_STEP) {
		_mapPoints.emplace(0, 0, z);
		_mapPoints.emplace(BOX_MAX_SIZE.x - 1, 0, z);
	}
}

bool	Terrain::draw(bool wireframe) {
	_sh->use();

	// update uniforms
	_sh->setMat4("view", _gui.cam->getViewMatrix());
	_sh->setVec3("viewPos", _gui.cam->pos);

	glBindVertexArray(_vao);
	if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// draw terrain surface
	glDrawElements(GL_TRIANGLE_STRIP, _indices.size(), GL_UNSIGNED_INT, 0);
	// draw terrain border
	glBindVertexArray(_vaoB);
	glDrawElements(GL_TRIANGLE_STRIP, _indicesB.size(), GL_UNSIGNED_INT, 0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // reset polygon mode
	glBindVertexArray(0);

	_sh->unuse();

	_water->draw(wireframe);

	return true;
}

float	Terrain::_calculateHeight(glm::uvec2 pos) {
	// is pos outside the terrain limit ?
	if (pos.x > BOX_MAX_SIZE.x || pos.y > BOX_MAX_SIZE.z) {
		logErr(std::string("[_calculateHeight] pos " + glm::to_string(pos) +
			" is outside the terrain limit " + glm::to_string(BOX_MAX_SIZE)).c_str());
	}

	// we already know pos height
	for (const glm::vec3 & p: _mapPoints) {
		if (glm::uvec2(p.x, p.z) == pos)
			return p.y;
	}

	/* we need to interpolate the height */
	std::vector<HeightPoint> closPoints = _getNClosest(pos, NB_CLOSEST_POINTS);

	// inverse distance weighting
	float top = 0;
	float bottom = 0;
	for (HeightPoint heightP : closPoints) {
		float distPow = heightP.distance;  // power of 1
		distPow *= distPow;  // power of 2
		distPow *= distPow;  // power of 3

		top += heightP.height / distPow;
		bottom += 1.0 / distPow;
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
	for (const glm::vec3 & p: _mapPoints) {
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

	// hL
	if (x != 0)
		hL = TERRAIN_H(x - 1, z);
	else
		hL = TERRAIN_H(x, z);
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
	if (z != 0)
		hB = TERRAIN_H(x, z - 1);
	else
		hB = TERRAIN_H(x, z);

	float sx = hR - hL;
	if (x == 0 || x == BOX_MAX_SIZE.x - 1)
		sx *= 2;

	float sy = hB - hT;
	if (z == 0 || z == BOX_MAX_SIZE.z -1)
		sy *= 2;

	glm::vec3 norm(-sx, 2.0, sy);

	return glm::normalize(norm);
}

bool	Terrain::init() {
	if (!_initMesh())
		return false;
	if (!_initMeshBorder())
		return false;
	if (!_water->init())
		return false;
	return true;
}

bool	Terrain::_initMesh() {
	// fill vertices
	for (uint16_t z = 0; z < BOX_MAX_SIZE.z; ++z) {
		for (uint16_t x = 0; x < BOX_MAX_SIZE.x; ++x) {
			TerrainVert	vert;
			float pX = x / (BOX_MAX_SIZE.x - 1) * BOX_MAX_SIZE.x;
			float pZ = z / (BOX_MAX_SIZE.z - 1) * BOX_MAX_SIZE.z;

			// force border to have null altitude
			if (x == 0 || x == BOX_MAX_SIZE.x - 1 || z == 0 || z == BOX_MAX_SIZE.z - 1)
				vert.pos = {pX, 0, pZ};
			else
				vert.pos = {pX, _calculateHeight({x, z}), pZ};
			_vertices.push_back(vert);
		}
	}

	// fill vert colors
	_initColors();

	// calc vertices normals
	for (TerrainVert & vert : _vertices) {
		uint16_t x = vert.pos.x / BOX_MAX_SIZE.x * (BOX_MAX_SIZE.x - 1);
		uint16_t z = vert.pos.z / BOX_MAX_SIZE.z * (BOX_MAX_SIZE.z - 1);
		vert.norm = _calculateNormal(x, z);
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(uint32_t),
		&_indices[0], GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVert),
		reinterpret_cast<void *>(0));
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVert),
		reinterpret_cast<void *>(offsetof(TerrainVert, norm)));
	// vertex colors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVert),
		reinterpret_cast<void *>(offsetof(TerrainVert, color)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	_staticUniform();

	return true;
}

bool	Terrain::_initMeshBorder() {
	std::array<glm::vec2, 8> bPos = {
		glm::vec2(0,				0),
		glm::vec2(BOX_MAX_SIZE.x,	0),
		glm::vec2(BOX_MAX_SIZE.x,	0),
		glm::vec2(BOX_MAX_SIZE.x,	BOX_MAX_SIZE.z),
		glm::vec2(BOX_MAX_SIZE.x,	BOX_MAX_SIZE.z),
		glm::vec2(0,				BOX_MAX_SIZE.z),
		glm::vec2(0,				BOX_MAX_SIZE.z),
		glm::vec2(0,				0)
	};

	std::array<glm::vec2, 8> bNorm = {
		glm::vec2(0, -1),
		glm::vec2(1, 0),
		glm::vec2(0, 1),
		glm::vec2(-1, 0),
	};

	// fill vertices
	float meshWidth = 8;
	_verticesB = std::vector<TerrainVert>(meshWidth * 2, TerrainVert());
	TerrainVert vert;
	for (int32_t i = 0; i < meshWidth; ++i) {
		vert.pos = {bPos[i][0], 0.0, bPos[i][1]};
		vert.norm = {bNorm[i / 2][0], 0, bNorm[i / 2][1]};
		vert.color = _borderColor;
		_verticesB[i] = vert;
		vert.pos = {vert.pos.x, -BOX_GROUND_HEIGHT, vert.pos.z};
		_verticesB[i + meshWidth] = vert;
	}

	// fill indices
	// We create a triangle strip to draw all the borders in one call
	uint32_t a, b;
	_indicesB.push_back(0);
	for (uint16_t x = 0; x < meshWidth; ++x) {
		a = x;
		b = a + meshWidth;
		_indicesB.push_back(a);
		_indicesB.push_back(b);
	}

	// create vao, vbo, ebo
	glGenVertexArrays(1, &_vaoB);
	glGenBuffers(1, &_vboB);
	glGenBuffers(1, &_eboB);

	// fill vao buffer
	glBindVertexArray(_vaoB);
	glBindBuffer(GL_ARRAY_BUFFER, _vboB);
	glBufferData(GL_ARRAY_BUFFER, _verticesB.size() * sizeof(TerrainVert),
		&_verticesB[0], GL_STATIC_DRAW);

	// set-up ebo
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _eboB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indicesB.size() * sizeof(uint32_t),
		&_indicesB[0], GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVert),
		reinterpret_cast<void *>(0));
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVert),
		reinterpret_cast<void *>(offsetof(TerrainVert, norm)));
	// vertex colors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVert),
		reinterpret_cast<void *>(offsetof(TerrainVert, color)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
}

void	Terrain::_initColors() {
	// calc min/max height
	_minH = _vertices[0].pos.y;
	_maxH = _minH;
	for (TerrainVert & vert : _vertices) {
		if (vert.pos.y < _minH)
			_minH = vert.pos.y;
		if (vert.pos.y > _maxH)
			_maxH = vert.pos.y;
	}

	// apply color based on the height ratio and the colors gradient array
	float diffH = _maxH - _minH;
	for (TerrainVert & vert : _vertices) {
		float ratio = (vert.pos.y - _minH) / diffH;
		vert.color = _calcColor(ratio);
	}

	// init border color
	float ratio = -_minH / diffH;
	_borderColor = _calcColor(ratio);
}

glm::vec3	Terrain::_calcColor(float ratio) {
	float step = 1.0 / (_colors.size() - 1);

	for (uint8_t i = 0; i < _colors.size() - 1; ++i) {
		if (ratio <= step * (i + 1)) {
			// modify ratio to be between 0 -> 1
			float minR = step * i;
			float maxR = step * (i + 1);
			float diffR = maxR - minR;
			ratio = (ratio - minR) / diffR;

			return glm::lerp(_colors[i], _colors[i+1], ratio);
		}
	}
	return {0, 0, 0};
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

bool	Terrain::update(float dtTime) {
	return _water->update(dtTime);
}

void	Terrain::setScenario(uint16_t scenarioId) {
	_water->setScenario(scenarioId);
}

// -- getters ------------------------------------------------------------------
float	Terrain::getHeight(uint32_t u, uint32_t v) const {
	return TERRAIN_H(u, v);
}

bool	Terrain::getNearHeight(float u, float v, float & height) const {
	int32_t x = std::round(u);
	int32_t z = std::round(v);
	if (x >= 0 && x < BOX_MAX_SIZE.x && z >= 0 && z < BOX_MAX_SIZE.z) {
		height = TERRAIN_H(x, z);
		return true;
	}
	return false;
}

float	Terrain::getMinHeight() const { return _minH; }
float	Terrain::getMaxHeight() const { return _maxH; }
float	Terrain::getOrbitDistance() const { return _scene.getOrbitDistance(); }

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

std::array<glm::vec3, 3>	Terrain::_colors = {
	glm::vec3(0.92f, 0.85f, 0.69f),  // #EBD9B1
	glm::vec3(0.61f, 0.76f, 0.2f),  // #9CC335
	glm::vec3(0.54f, 0.5f, 0.56f)  // #8A808F
};
