#ifndef TERRAIN_HPP_
#define TERRAIN_HPP_

#include <string>

#include "mod1.hpp"

class Terrain {
	public:
		Terrain(std::string mapPath);
		virtual ~Terrain();
		Terrain(Terrain const &src);
		Terrain &operator=(Terrain const &rhs);

		// -- exceptions -------------------------------------------------------
		/**
		 * @brief Terrain exception
		 */
		class TerrainException : public std::runtime_error {
			public:
				TerrainException();
				explicit TerrainException(const char* what_arg);
		};

	private:
		void	_loadFile();

		std::string		_mapPath;
		SettingsJson	*_map;
};

#endif  // TERRAIN_HPP_
