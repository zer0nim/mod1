#ifndef SCENE_HPP_
#define SCENE_HPP_

#include <vector>

#include "mod1.hpp"
#include "Gui.hpp"
#include "Terrain.hpp"
#include "Water.hpp"
#include "OrbitControls.hpp"
#include "InfosUI.hpp"

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
		uint16_t	getTerrainId() const;
		uint16_t	getScenarioId() const;
		bool	getPause() const;

	private:
		bool	_update();
		bool	_draw();

		std::vector<Terrain *> &	_terrains;
		Gui		_gui;
		float	_dtTime;
		float	_fps;
		bool	_wireframeMode;
		int32_t	_terrainId;
		int32_t	_scenarioId;
		OrbitControls	* _orbitControls;
		bool	_pause;
		InfosUI *	_infosUI;
};

#endif  // SCENE_HPP_
