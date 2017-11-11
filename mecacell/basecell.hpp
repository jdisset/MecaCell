#ifndef MECACELL_BASECELL_HPP
#define MECACELL_BASECELL_HPP

/*----------------------------------------------------------------------------*\
|																			   |
|      						   MecaCell::BaseCell							   |
| 																			   |
\*----------------------------------------------------------------------------*/
/**
 * @brief Basis for every cell a user might want to use.
 *
 * It uses the curriously reccuring template pattern : Derived is the user's type
 * inheriting BaseCell.
 * Body is the type of body used by the cell. A body is a low level class
 * that handles everything related to physics (deformations, connections, volume).
 * A BaseCell and the classes that inherit from it is a more abstract and general
 * interface meant to expose higher instinctive, meaningful parameters and methods.
 *
 * @tparam Body the body implementation (which contains most of the core logic)
 */
template <class Derived, template <class> class Body = ContactSurfaceBody>
class BaseCell {
 public:
	using body_t = Body<Derived>;
	using embedded_plugin_t = typename body_t::embedded_plugin_t;
	using vec_t = decltype((body_t) nullptr->getPosition());

	friend body_t;
	friend embedded_plugin_t;

 protected:
	body_t body;        // body contains cell physics implementation
	bool dead = false;  // when a cell is dead it is flagged for destruction

 public:
	/*----------------------------------------------------------------------------*\
	|   	 			 		  	 CONSTRUCTORS								   |
	\*----------------------------------------------------------------------------*/
	BaseCell(const Derived &c) : body(static_cast<Derived *>(this)) {}
	BaseCell(const vec_t &pos) : body(static_cast<Derived *>(this), pos) {}

	/*----------------------------------------------------------------------------*\
	|   	 			   				 ID										   |
	\*----------------------------------------------------------------------------*/
	// A cell should have a unique id in its world, mostly for determinism & debugging
	// purposes. Usually matches the order of insertion in the world.
	size_t id = 0;
	size_t getId() { return id; }

	/*----------------------------------------------------------------------------*\
	|   	 	   				 		UTILITIES								   |
	\*----------------------------------------------------------------------------*/
	void die() { dead = true; }     // flag the cell for destruction
	bool isDead() { return dead; }  // simple getter
	vec_t getPosition() { return body.getPosition(); }
};

#endif
