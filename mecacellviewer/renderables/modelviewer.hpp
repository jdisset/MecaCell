#ifndef MECACELLVIEWER_MODELVIEWER_HPP
#define MECACELLVIEWER_MODELVIEWER_HPP
#include <string>
#include <unordered_map>
#include "model.hpp"

namespace MecacellViewer {
template <typename M, typename R> struct ModelViewer : public PaintStep<R> {
	std::unordered_map<std::string, Model<M>> models;
	ModelViewer() : PaintStep<R>("Models") {}
	void call(R *r) {
		for (auto &m : r->getScenario().getWorld().models) {
			if (!models.count(m.first)) {
				models[m.first].load(m.second);
			}
			models[m.first].draw(r->getViewMatrix(), r->getProjectionMatrix(), m.second);
		}
	}
};
}
#endif
