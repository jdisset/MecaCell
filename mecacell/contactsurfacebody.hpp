#ifndef CONTACTSURFACEBODY_HPP
#define CONTACTSURFACEBODY_HPP
namespace MecaCell {

template <typename Cell> class ContactSurfaceBody {
	Cell* cell = nullptr;
	double radius = 10;

 public:
	using embedded_plugin_t = char;

	ContactSurfaceBody(Cell* c) : cell(c){};
	double getBoundingBoxRadius() const { return radius; }
};
}
#endif
