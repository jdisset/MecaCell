#ifndef MECACELLVIEWER_MESH_VIEWER_PLUGIN_HPP
#define MECACELLVIEWER_MESH_VIEWER_PLUGIN_HPP
#include <mecacell/utilities/obj3D.hpp>
#include "../primitives/model.hpp"

namespace MecacellViewer {

/**
 * @brief PLugin that displays a MecaCell::Scene3D
 *  THIS IS A VIEWER PLUGIN. Instanciate where you want and register with
 * Viewer::registerPlugin
 */

struct MeshViewerPlugin {
	std::unordered_map<std::string, MecacellViewer::Mesh> meshes;
	MecaCell::Scene3D* scene;
	/**
	 * @brief Constructor
	 *
	 * @param s a pointer to the 3D scene to be rendered
	 */
	MeshViewerPlugin(MecaCell::Scene3D* s) : scene(s) {}

	template <typename R> void onLoad(R* renderer) {
		std::cerr << "MeshViewerPlugin = " << this << std::endl;
		for (auto& o : scene->transformedObjects) {
			std::cerr << "loading " << o.first << std::endl;
			std::cerr << "scene = " << scene << ", renderer = " << renderer << std::endl;
			meshes[o.first].load(o.second.vertices, o.second.uv, o.second.normals,
			                     o.second.faces);
			std::cerr << "loaded ok" << std::endl;
		}
		renderer->addPaintStepsMethods(1000, [this](auto* r) { draw(r); });
	}
	template <typename R> void draw(R* r) {
		for (auto& m : meshes)
			m.second.draw(r->getViewMatrix(), r->getProjectionMatrix(), QMatrix4x4());
	}
};
}
#endif
