#ifndef SCENE_HPP_
#define SCENE_HPP_

#include "mod1.hpp"
#include "Gui.hpp"

class Scene {
	public:
		Scene();
		virtual ~Scene();
		Scene(Scene const &src);
		Scene &operator=(Scene const &rhs);

		bool	init();
		bool	run();
		float	getDtTime() const;
		uint16_t	getFps() const;
	private:
		bool	_update();
		bool	_draw();

		Gui		_gui;
		float	_dtTime;
		float	_fps;
};

#endif  // SCENE_HPP_
