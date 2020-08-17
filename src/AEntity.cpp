#include "AEntity.hpp"
#include "Model.hpp"
#include "BoxCollider.hpp"

// -- Constructors -------------------------------------------------------------

/**
 * @brief Construct a new AEntity::AEntity object
 */
AEntity::AEntity() {
	size = {1, 1, 1};
	active = true;
	position = VOID_POS3;
	name = "Entity";
	_model = nullptr;
}

/**
 * @brief Destroy the AEntity::AEntity object
 */
AEntity::~AEntity() {
	delete _model;
}

/**
 * @brief Construct a new AEntity::AEntity object
 *
 * @param src A AEntity element to copy
 */
AEntity::AEntity(AEntity const &src) : AEntity() {
	*this = src;
}

// -- Methods ------------------------------------------------------------------

/**
 * @brief Init method
 *
 * @return true on success
 * @return false on failure
 */
bool		AEntity::init() {
	return true;
}

/**
 * @brief Update method
 *
 * @return true on success
 * @return false on failure
 */
bool		AEntity::update() {
	return true;
}

/**
 * @brief Draw the collider of the entity
 *
 * @return false If failed
 */
bool		AEntity::drawCollider() {
	glm::vec4 color = colorise(s.j("colors").j("collider").u("color"), s.j("colors").j("collider").u("alpha"));
	BoxCollider::drawBox(position, size, color);
	return true;
}

/**
 * @brief called on animation end if passed to Model
 * need to be redefined by children
 *
 * @param animName the current animation name
 */
void	AEntity::animEndCb(std::string animName) {
	(void)animName;
	return;
}

// -- Operators ----------------------------------------------------------------

/**
 * @brief Overloaded operator
 *
 * @param rhs Right element
 * @return AEntity& A ref to a new object
 */
AEntity &AEntity::operator=(AEntity const &rhs) {
	if ( this != &rhs ) {
		active = rhs.active;
		name = rhs.name;
		position = rhs.position;
		_model = nullptr;

		// if exist, copy the model
		if (rhs._model) {
			init();  // create new model
			*_model = *rhs._model;  // restore model settings
		}
	}
	return *this;
}
