#include <cmath>

#include "Water.hpp"

// -- const --------------------------------------------------------------------
// space between grid points
glm::vec2 const	Water::_gridSpace = glm::vec2(
	BOX_MAX_SIZE.x / WATER_GRID_RES.x, BOX_MAX_SIZE.z / WATER_GRID_RES.y);
// water grid pipe length
glm::vec2 const	Water::_pipeLen = _gridSpace;

// grid box area
float const	Water::_gridArea = Water::_gridSpace.x * Water::_gridSpace.y;

std::unique_ptr<Shader> Water::_sh = nullptr;

// -- members ------------------------------------------------------------------
Water::Water(Terrain & terrain, Gui & gui)
: _gui(gui),
  _terrain(terrain),
  _vao(0),
  _vbo(0),
  _ebo(0),
  _vaoB(0),
  _vboB(0),
  _eboB(0) {
	// init static shader if null
	if (!_sh) {
		_sh = std::unique_ptr<Shader>(
			new Shader("shaders/water_vs.glsl", "shaders/water_fs.glsl"));
	}

	_gravity = 9.81;
	// allocate _waterCols 2d array
	_waterCols = std::vector< std::vector<WaterColum> >(
		WATER_GRID_RES.y, std::vector<WaterColum>(WATER_GRID_RES.x, WaterColum()));
}

Water::~Water() {
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
}

Water::Water(Water const &src)
: _gui(src._gui),
  _terrain(src._terrain){
	*this = src;
}

Water &Water::operator=(Water const &rhs) {
	if (this != &rhs) {
		logWarn("operator= called");
	}
	return *this;
}

bool	Water::init() {
	// init column terrain height now to avoid retrieving it every update
	for (uint32_t v = 0; v < WATER_GRID_RES.y; ++v) {
		for (uint32_t u = 0; u < WATER_GRID_RES.x; ++u) {
			uint32_t tU = (uint32_t)((u + 0.5) * _gridSpace.x);
			uint32_t tV = (uint32_t)((v + 0.5) * _gridSpace.y);
			_waterCols[v][u].terrainH = _terrain.getHeight(tU, tV);

			// init left waters column at 0 to test
			if (u == 0)
				_waterCols[v][u].depth = 30.0;
		}
	}

	if (!_initMesh())
		return false;
	if (!_initMeshBorder())
		return false;
	return true;
}

bool	Water::update(float dtTime) {
	// update all water columns
	for (uint32_t v = 0; v < WATER_GRID_RES.y; ++v) {
		for (uint32_t u = 0; u < WATER_GRID_RES.x; ++u) {
			_updateFlow(u, v, dtTime);
		}
	}

	// scale flow to prevent negative water depth
	_correctNegWaterDepth(dtTime);

	// update all water columns
	for (uint32_t v = 0; v < WATER_GRID_RES.y; ++v) {
		for (uint32_t u = 0; u < WATER_GRID_RES.x; ++u) {
			_updateDepth(u, v, dtTime);
		}
	}

	// update the mesh accordingly
	if (!_updateMesh())
		return false;
	if (!_updateMeshBorder())
		return false;

	return true;
}

void	Water::_updateFlow(uint32_t u, uint32_t v, float dtTime) {
	/*
		pipeCSA is the cross-sectional area of the pipe
		Artificially varying pipeCSA leads to an approximate method for modeling viscosity
		(larger values make the water more lively).
	*/
	float pipeCSA = _gridArea;

	// column water total height
	float totalH = _waterCols[v][u].terrainH + _waterCols[v][u].depth;

	/* left flow */
	// if there is a wall, set the flow to 0
	// verify area limit or terain wall
	bool wall = u == 0;
	float totalHLeft = 0.0;
	if (!wall) {
		totalHLeft = _waterCols[v][u - 1].terrainH + _waterCols[v][u - 1].depth;
		bool wallLeft = _waterCols[v][u - 1].depth == 0 && _waterCols[v][u - 1].terrainH > totalH;
		bool wallRight = _waterCols[v][u].depth == 0 && _waterCols[v][u].terrainH > totalHLeft;
		wall = wallLeft || wallRight;
	}
	if (wall) {
		_waterCols[v][u].lFlow = 0.0;
	}
	else {
		float hDiff = 0.0;
		float freeWaterH = 0.0;
		if (totalH > totalHLeft) {
			float totalHDiff = totalH - totalHLeft;
			freeWaterH = totalHDiff > _waterCols[v][u].depth ? _waterCols[v][u].depth : totalHDiff;
			hDiff = -freeWaterH;
		}
		else {
			float totalHDiff = totalHLeft - totalH;
			freeWaterH = totalHDiff > _waterCols[v][u - 1].depth ? _waterCols[v][u - 1].depth : totalHDiff;
			hDiff = freeWaterH;
		}
		pipeCSA = _gridSpace.x * freeWaterH;

		_waterCols[v][u].lFlow += pipeCSA * (_gravity / _pipeLen.x) * hDiff * dtTime;
	}

	/* top flow */
	// if there is a wall, set the flow to 0
	// verify area limit or terain wall
	wall = v == 0;
	float totalHTop = 0.0;
	if (!wall) {
		totalHTop = _waterCols[v - 1][u].terrainH + _waterCols[v - 1][u].depth;
		bool wallTop = _waterCols[v - 1][u].depth == 0 && _waterCols[v - 1][u].terrainH > totalH;
		bool wallBottom = _waterCols[v][u].depth == 0 && _waterCols[v][u].terrainH > totalHTop;
		wall = wallTop || wallBottom;
	}
	if (wall) {
		_waterCols[v][u].tFlow = 0.0;
	}
	else {
		float hDiff = 0.0;
		float freeWaterH = 0.0;
		if (totalH > totalHTop) {
			float totalHDiff = totalH - totalHTop;
			freeWaterH = totalHDiff > _waterCols[v][u].depth ? _waterCols[v][u].depth : totalHDiff;
			hDiff = -freeWaterH;
		}
		else {
			float totalHDiff = totalHTop - totalH;
			freeWaterH = totalHDiff > _waterCols[v - 1][u].depth ? _waterCols[v - 1][u].depth : totalHDiff;
			hDiff = freeWaterH;
		}
		pipeCSA = _gridSpace.y * freeWaterH;

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
	// prevent the depth from going bellow 0
	_waterCols[v][u].depth = std::max(0.0f, _waterCols[v][u].depth);
}

void	Water::_correctNegWaterDepth(float dtTime) {
	bool asNegDepth = true;
	for (uint16_t i = 0; asNegDepth && i < 5; ++i) {
		asNegDepth = false;
		// search for negativ depth and correct them
		for (uint32_t v = 0; v < WATER_GRID_RES.y; ++v) {
			for (uint32_t u = 0; u < WATER_GRID_RES.x; ++u) {
				float lFlow = 0, tFlow = 0, rFlow = 0, bFlow = 0;
				float totNegFlow = 0;  // store the total negative flow
				float totPosFlow = 0;  // store the total negative flow

				// left flow
				lFlow = _waterCols[v][u].lFlow;
				if (lFlow < 0)
					totNegFlow += lFlow;
				else
					totPosFlow += lFlow;
				// top flow
				tFlow = _waterCols[v][u].tFlow;
				if (tFlow < 0)
					totNegFlow += tFlow;
				else
					totPosFlow += tFlow;
				// right flow
				if (u < WATER_GRID_RES.x - 1) {
					rFlow = -_waterCols[v][u + 1].lFlow;
					if (rFlow < 0)
						totNegFlow += rFlow;
					else
						totPosFlow += rFlow;
				}
				// bottom flow
				if (v < WATER_GRID_RES.y - 1) {
					bFlow = -_waterCols[v + 1][u].tFlow;
					if (bFlow < 0)
						totNegFlow += bFlow;
					else
						totPosFlow += bFlow;
				}

				float totalFlow = lFlow + tFlow + rFlow + bFlow;
				float depthChange = (totalFlow / _gridArea) * dtTime;
				float newDepth = _waterCols[v][u].depth + depthChange;

				// if the new depth is negative, scale down the negative flow
				if (newDepth < 0) {
					asNegDepth = true;

					double totPosDepth = (totPosFlow / _gridArea) * dtTime;
					double desNegDepth = -_waterCols[v][u].depth + totPosDepth;
					double corNegFlow = (desNegDepth * _gridArea) / dtTime;
					double corRatio = corNegFlow / totNegFlow;

					if (lFlow < 0)
						_waterCols[v][u].lFlow *= corRatio;
					if (tFlow < 0)
						_waterCols[v][u].tFlow *= corRatio;
					if (rFlow < 0)
						_waterCols[v][u + 1].lFlow *= corRatio;
					if (bFlow < 0)
						_waterCols[v + 1][u].tFlow *= corRatio;
				}
			}
		}
	}
}

bool	Water::draw(bool wireframe) {
	_sh->use();

	// update uniforms
	_sh->setMat4("view", _gui.cam->getViewMatrix());
	_sh->setVec3("viewPos", _gui.cam->pos);

	glBindVertexArray(_vao);
	if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// enable skybox texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _gui.getSkybox().getTextureID());

	// draw water surface
	_sh->setVec4("wColor", 0.42, 0.58, 0.65, 0.5);  // #6d94a6
	_sh->setBool("skyReflection", true);
	glDrawElements(GL_TRIANGLE_STRIP, _indices.size(), GL_UNSIGNED_INT, 0);
	// draw water border
	glBindVertexArray(_vaoB);
	_sh->setVec4("wColor", 0.42, 0.58, 0.65, 0.6);  // #6d94a6
	_sh->setBool("skyReflection", false);
	glDrawElements(GL_TRIANGLE_STRIP, _indicesB.size(), GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);  // disable skybox texture
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // reset polygon mode
	glBindVertexArray(0);

	_sh->unuse();

	return true;
}

bool	Water::_initMesh() {
	// fill vertices
	for (uint16_t z = 0; z < WATER_GRID_RES.y + 1; ++z) {
		for (uint16_t x = 0; x < WATER_GRID_RES.x + 1; ++x) {
			WaterVert vert;
			bool noWater = true;
			vert.pos = {_gridSpace.x * x, _calculateHeight(x, z, noWater), _gridSpace.y * z};
			vert.visible = noWater ? 0.0 : 1.0;
			_vertices.push_back(vert);
		}
	}

	// calc vertices normals
	for (WaterVert & vert : _vertices) {
		uint16_t x = std::round(vert.pos.x / _gridSpace.x);
		uint16_t z = std::round(vert.pos.z / _gridSpace.y);
		vert.norm = _calculateNormal(x, z);
	}

	// fill indices
	// By repeating the last vertex and the first vertex, we created four
	// degenerate triangles that will be skipped, and linked the first row
	// with the second. We could link an arbitrary number of rows this way and
	// draw the entire mesh with only one call
	// cf: learnopengles.com/tag/triangle-strips
	for (uint16_t y = 0; y < WATER_GRID_RES.y; ++y) {
		// duplicate first vertice to generate degenerate triangle
		if (y > 0)
			_indices.push_back(y * (WATER_GRID_RES.x + 1));

		uint32_t a = 0;
		uint32_t b = 0;
		for (uint16_t x = 0; x < WATER_GRID_RES.x + 1; ++x) {
			a = x + y * (WATER_GRID_RES.x + 1);
			b = a + (WATER_GRID_RES.x + 1);
			_indices.push_back(a);
			_indices.push_back(b);
		}

		// duplicate last vertice to generate degenerate triangle
		if (y != WATER_GRID_RES.y - 1)
			_indices.push_back(b);
	}

	// create vao, vbo, ebo
	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);
	glGenBuffers(1, &_ebo);

	// fill vao buffer
	glBindVertexArray(_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(WaterVert),
		&_vertices[0], GL_STATIC_DRAW);

	// set-up ebo
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(u_int32_t),
		&_indices[0], GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(WaterVert),
		reinterpret_cast<void *>(0));
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(WaterVert),
		reinterpret_cast<void *>(offsetof(WaterVert, norm)));
	// vertex visibility
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(WaterVert),
		reinterpret_cast<void *>(offsetof(WaterVert, visible)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	_staticUniform();

	return true;
}

bool	Water::_updateMesh() {
	// update vertices pos/normals/visibility
	bool noWater = true;
	for (WaterVert & vert : _vertices) {
		uint16_t x = std::round(vert.pos.x / _gridSpace.x);
		uint16_t z = std::round(vert.pos.z / _gridSpace.y);
		vert.pos = glm::vec3(vert.pos.x, _calculateHeight(x, z, noWater), vert.pos.z);
		vert.visible = noWater ? 0.0 : 1.0;
		vert.norm = _calculateNormal(x, z);
	}

	// update vbo data
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, _vertices.size() * sizeof(WaterVert), &_vertices[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

bool	Water::_initMeshBorder() {
	// fill vertices
	float meshWidth = (WATER_GRID_RES.x + 1) * 2 + (WATER_GRID_RES.y + 1) * 2;
	_verticesB = std::vector<WaterVert>(meshWidth * 2, WaterVert());
	_updateBorderVertices();

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
	glBufferData(GL_ARRAY_BUFFER, _verticesB.size() * sizeof(WaterVert),
		&_verticesB[0], GL_STATIC_DRAW);

	// set-up ebo
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _eboB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indicesB.size() * sizeof(u_int32_t),
		&_indicesB[0], GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(WaterVert),
		reinterpret_cast<void *>(0));
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(WaterVert),
		reinterpret_cast<void *>(offsetof(WaterVert, norm)));
	// vertex visibility
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(WaterVert),
		reinterpret_cast<void *>(offsetof(WaterVert, visible)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
}

bool	Water::_updateMeshBorder() {
	_updateBorderVertices();

	// update vbo data
	glBindBuffer(GL_ARRAY_BUFFER, _vboB);
	glBufferSubData(GL_ARRAY_BUFFER, 0, _verticesB.size() * sizeof(WaterVert), &_verticesB[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

void	Water::_updateBorderVertices() {
	// update vertices pos/normals/visibility
	float meshWidth = (WATER_GRID_RES.x + 1) * 2 + (WATER_GRID_RES.y + 1) * 2;
	uint32_t i = 0;
	WaterVert vert;
	for (int32_t x = 0; x < WATER_GRID_RES.x + 1; ++x) {
		bool noWater = true;
		float depth = _calculateHeight(x, 0, noWater);
		vert.pos = {x * _gridSpace.x, depth, 0};
		vert.norm = {0, 0, -1};
		vert.visible = noWater ? 0.0 : 1.0;
		_verticesB[i] = vert;
		vert.pos = {vert.pos.x, 0.0, vert.pos.z};
		_verticesB[i + meshWidth] = vert;
		++i;
	}
	for (int32_t z = 0; z < WATER_GRID_RES.y + 1; ++z) {
		bool noWater = true;
		float depth = _calculateHeight(WATER_GRID_RES.x, z, noWater);
		vert.pos = {WATER_GRID_RES.x * _gridSpace.x, depth, z * _gridSpace.y};
		vert.norm = {1, 0, 0};
		vert.visible = noWater ? 0.0 : 1.0;
		_verticesB[i] = vert;
		vert.pos = {vert.pos.x, 0.0, vert.pos.z};
		_verticesB[i + meshWidth] = vert;
		++i;
	}
	for (int32_t x = WATER_GRID_RES.x; x >= 0; --x) {
		bool noWater = true;
		float depth = _calculateHeight(x, WATER_GRID_RES.y, noWater);
		vert.pos = {x * _gridSpace.x, depth, WATER_GRID_RES.y * _gridSpace.y};
		vert.norm = {0, 0, 1};
		vert.visible = noWater ? 0.0 : 1.0;
		_verticesB[i] = vert;
		vert.pos = {vert.pos.x, 0.0, vert.pos.z};
		_verticesB[i + meshWidth] = vert;
		++i;
	}
	for (int32_t z = WATER_GRID_RES.y; z >= 0; --z) {
		bool noWater = true;
		float depth = _calculateHeight(0, z, noWater);
		vert.pos = {0, depth, z * _gridSpace.y};
		vert.norm = {-1, 0, 0};
		vert.visible = noWater ? 0.0 : 1.0;
		_verticesB[i] = vert;
		vert.pos = {vert.pos.x, 0.0, vert.pos.z};
		_verticesB[i + meshWidth] = vert;
		++i;
	}
}

float	Water::_calculateHeight(uint32_t x, uint32_t z, bool & noWater) {
	float hL, hR, hB, hT;

	noWater = true;
	uint32_t zHoriz = z < WATER_GRID_RES.y ? z : z - 1;
	// hL
	if (x != 0) {
		hL = _waterCols[zHoriz][x - 1].depth;
		noWater = noWater && (hL == 0);
		hL += _waterCols[zHoriz][x - 1].terrainH;
	}
	else {
		hL = _waterCols[zHoriz][x].depth;
		noWater = noWater && (hL == 0);
		hL += _waterCols[zHoriz][x].terrainH;
	}
	// hR
	if (x < WATER_GRID_RES.x) {
		hR = _waterCols[zHoriz][x].depth;
		noWater = noWater && (hR == 0);
		hR += _waterCols[zHoriz][x].terrainH;
	}
	else {
		hR = _waterCols[zHoriz][x - 1].depth;
		noWater = noWater && (hR == 0);
		hR += _waterCols[zHoriz][x - 1].terrainH;
	}
	uint32_t xVert = x < WATER_GRID_RES.x ? x : x - 1;
	// hT
	if (z != 0) {
		hB = _waterCols[z - 1][xVert].depth;
		noWater = noWater && (hB == 0);
		hB += _waterCols[z - 1][xVert].terrainH;
	}
	else {
		hB = _waterCols[z][xVert].depth;
		noWater = noWater && (hB == 0);
		hB += _waterCols[z][xVert].terrainH;
	}
	// hB
	if (z < WATER_GRID_RES.y) {
		hT = _waterCols[z][xVert].depth;
		noWater = noWater && (hT == 0);
		hT += _waterCols[z][xVert].terrainH;
	}
	else {
		hT = _waterCols[z - 1][xVert].depth;
		noWater = noWater && (hT == 0);
		hT += _waterCols[z - 1][xVert].terrainH;
	}

	return (hL + hR + hB + hT) / 4.0;
}

glm::vec3	Water::_calculateNormal(uint32_t x, uint32_t z) {
	float hL, hR, hB, hT;

	// hL
	if (x != 0)
		hL = WATER_H(x - 1, z);
	else
		hL = WATER_H(x, z);
	// hR
	if (x < WATER_GRID_RES.x)
		hR = WATER_H(x + 1, z);
	else
		hR = WATER_H(x, z);
	// hT
	if (z < WATER_GRID_RES.y)
		hT = WATER_H(x, z + 1);
	else
		hT = WATER_H(x, z);
	// hB
	if (z != 0)
		hB = WATER_H(x, z - 1);
	else
		hB = WATER_H(x, z);

	float sx = hR - hL;
	if (x == 0 || x == WATER_GRID_RES.x)
		sx *= 2;

	float sy = hB - hT;
	if (z == 0 || z == WATER_GRID_RES.y)
		sy *= 2;

	glm::vec3 norm(-sx, 2.0, sy);

	return glm::normalize(norm);
}

void	Water::_staticUniform() {
	_sh->use();

	// water color
	_sh->setVec4("wColor", 0.098, 0.619, 0.901, 0.7);  // #199ee6

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

// -- WaterColum ---------------------------------------------------------------
WaterColum::WaterColum() {
	depth = 0.0;
	lFlow = 0.0;
	tFlow = 0.0;
	terrainH = 0.0;
}
