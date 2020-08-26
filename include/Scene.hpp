#ifndef SCENE_HPP_
#define SCENE_HPP_

#include <vector>

#include "mod1.hpp"
#include "Gui.hpp"
#include "Terrain.hpp"

class Scene {
	public:
		Scene(std::vector<Terrain *> & terrains);
		virtual ~Scene();
		Scene(Scene const &src);
		Scene &operator=(Scene const &rhs);

		bool	init();
		bool	run();
		Gui &	getGui();
		float	getDtTime() const;
		uint16_t	getFps() const;
	private:
		bool	_update();
		bool	_draw();

		std::vector<Terrain *> &	_terrains;
		Gui		_gui;
		float	_dtTime;
		float	_fps;
		bool	_wireframeMode;
		uint32_t	_terrainId;
};

#endif  // SCENE_HPP_
