#include <fstream>

#include "TextureManager.hpp"
#include "utils/opengl/Texture.hpp"
#include "Logging.hpp"

/**
 * @brief Construct a new Texture Manager:: Texture Manager object
 */
TextureManager::TextureManager()
: _textureAtlas(0) {
	// load texture atlas
	std::string path = "asset/textures/textures.png";
	bool inSpaceSRGB = true;
	try {
		_textureAtlas = textureAtlasFromFile(path, inSpaceSRGB, 32, 256);
	}
	catch(TextureException const & e) {
		throw TextureManagerException("failed to load texture atlas");
	}
}

/**
 * @brief Destroy the Texture Manager:: Texture Manager object
 */
TextureManager::~TextureManager() {
	// release openGl texture
	if (_textureAtlas != 0) {
		glDeleteTextures(1, &_textureAtlas);
	}
}

/**
 * @brief Construct a new Texture Manager:: Texture Manager object
 *
 * @param src The object to do the copy
 */
TextureManager::TextureManager(TextureManager const &src)
: _textureAtlas(0) {
	*this = src;
}

/**
 * @brief Copy this object
 *
 * @param rhs The object to copy
 * @return TextureManager& A reference to the copied object
 */
TextureManager &TextureManager::operator=(TextureManager const &rhs) {
	(void)rhs;
	return *this;
}

/**
 * @brief set the uniform for the textureAtlas in the shader
 *
 * @param sh the shader
 */
void	TextureManager::setUniform(Shader &sh) const {
	// activate textures
	sh.setInt("textureAtlas", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _textureAtlas);

	// set textures uniform
	uint8_t	i = 0;
	for (std::array<int8_t, 6> const &block : _blocks) {
		for (uint8_t j = 0; j < 6; ++j) {
			sh.setInt("blockTextures[" + std::to_string(i * 6 + j) + "]", block[j]);
		}
		++i;
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

/**
 * @brief bind the texture atlas
 */
void	TextureManager::activateTextures() const {
	// activate textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _textureAtlas);
}

/**
 * @brief unbind the texture atlas
 */
void	TextureManager::disableTextures() const {
	// disable textures
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

// -- exceptions ---------------------------------------------------------------
TextureManager::TextureManagerException::TextureManagerException()
: std::runtime_error("[TextureManagerException]") {}

TextureManager::TextureManagerException::TextureManagerException(const char* what_arg)
: std::runtime_error(std::string(std::string("[TextureManagerException] ") + what_arg).c_str()) {}

// -- statics const ------------------------------------------------------------
std::array<std::array<int8_t, 6>, NB_BLOCK_TYPES> const	TextureManager::_blocks = {{
	{{0, 0, 0, 0, 0, 0}}, // grass
	{{1, 1, 1, 1, 1, 1}}, // durable_wall
	{{2, 2, 2, 2, 2, 2}}, // destructible_wall
	{{3, 3, 3, 3, 3, 3}}, // prismarine
	{{4, 4, 4, 4, 4, 4}}, // stone_slab
	{{5, 5, 5, 5, 5, 5}}, // noteblock
	{{6, 6, 6, 6, 6, 6}}, // hardened_clay
	{{7, 7, 7, 7, 7, 7}}, // stonebrick_light
	{{8, 8, 8, 8, 8, 8}}, // stonebrick_dark
}};
