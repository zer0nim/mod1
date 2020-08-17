#ifndef TEXTUREMANAGER_HPP_
#define TEXTUREMANAGER_HPP_

#include <array>

#include "Shader.hpp"
#include "mod1.hpp"

#define NB_BLOCK_TYPES 9

namespace Block {
	/**
	 * @brief All blocks types
	 */
	enum Enum {
		GRASS,
		DURABLE_WALL,
		DESTRUCTIBLE_WALL,
		PRISMARINE,
		STONE_SLAB,
		NOTEBLOCK,
		HARDENED_CLAY,
		STONEBRICK_LIGHT,
		STONEBRICK_DARK,
	};
}  // namespace Block

/**
 * @brief This is the main class to manage textures
 */
class TextureManager {
	public:
		TextureManager();
		virtual ~TextureManager();
		TextureManager(TextureManager const &src);
		TextureManager &operator=(TextureManager const &rhs);

		void	setUniform(Shader &sh) const;
		void	activateTextures() const;
		void	disableTextures() const;

		// -- exceptions -------------------------------------------------------
		/**
		 * @brief TextureManager exception
		 */
		class TextureManagerException : public std::runtime_error {
			public:
				TextureManagerException();
				/**
				 * @brief Construct a new Texture Manager Exception object
				 *
				 * @param what_arg The error message
				 */
				explicit TextureManagerException(const char* what_arg);
		};

	private:
		uint32_t	_textureAtlas;  /**< Texture atlas */
		static std::array<std::array<int8_t, 6>, NB_BLOCK_TYPES> const	_blocks;  /**< All blocks */
};

#endif  // TEXTUREMANAGER_HPP_
