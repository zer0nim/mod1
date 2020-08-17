#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include <iostream>
#include <stdexcept>

#include "useGlm.hpp"
#include "Logging.hpp"
#include "Gui.hpp"
#include "ModelsManager.hpp"

class Model;
class SceneGame;

/**
 * @brief This is the base class for entity
 */
class AEntity {
protected:
	Model		*_model;  /**< The 3D model of the entity */
	glm::vec3	size;  /**< The entity size */

public:
	// Members
	bool		active;  /**< True if the entity is active */
	std::string	name;  /**< The entity name */
	glm::vec3	position;  /**< The entity position */

	// Constructors
	AEntity();
	virtual ~AEntity();
	AEntity(const AEntity &src);

	// Operators
	AEntity			&operator=(const AEntity &rhs);

	// Methods
	virtual bool		init();
	/**
	 * @brief Update entity. Called on every frames
	 *
	 * @return false If failed
	 */
	virtual bool		update();
	/**
	 * @brief Draw entity. Called on every frames
	 *
	 * @param gui A reference to the gui object
	 * @return false If failed
	 */
	virtual bool		draw(Gui &gui) = 0;
	virtual bool		drawCollider();

	virtual void		animEndCb(std::string animName);
};

#endif  // ENTITY_HPP_
