#ifndef GRID_HPP
#define GRID_HPP

#include <vector>
#include <iostream>
#include <unordered_map>
#include "tools.h"
using namespace std;

namespace MecaCell {
template <typename O> class Grid {
private:
	double cellSize; // actually it's 1/cellSize, just so we can multiply
	unordered_map<Vec, vector<O *>> um;

public:
	Grid(double cs) : cellSize(1.0 / cs) {}

	double getCellSize() const { return 1.0 / cellSize; }
	const unordered_map<Vec, vector<O *>> &getContent() const { return um; }

	void insert(O *obj) {
		Vec center = obj->getPosition() * cellSize;
		double radius = obj->getRadius() * cellSize;
		Vec minCorner = center - radius;
		Vec maxCorner = center + radius;
		minCorner.iterateTo(maxCorner, [&](Vec v) { um[v].push_back(obj); });
	}

	void insert(O *obj, const Vec &p0, const Vec &p1, const Vec &p2) { // insert triangles
	}

	vector<O *> retrieve(const Vec &coord, double r) const {
		vector<O *> res;
		Vec center = coord * cellSize;
		double radius = r * cellSize;
		Vec minCorner = center - radius;
		Vec maxCorner = center + radius;
		// TODO check if faster with a set (uniques...) and by removing  selfcollision
		minCorner.iterateTo(maxCorner, [&](const Vec &v) {
			if (um.count(v) > 0) res.insert(res.end(), um.at(v).begin(), um.at(v).end());
		});
		return res;
	}

	vector<O *> retrieve(O *obj) const {
		vector<O *> res;
		Vec center = obj->getPosition() * cellSize;
		double radius = obj->getRadius() * cellSize;
		Vec minCorner = center - radius;
		Vec maxCorner = center + radius;
		minCorner.iterateTo(maxCorner, [this, &res](const Vec &v) {
			if (um.count(v) > 0) res.insert(res.end(), um.at(v).begin(), um.at(v).end());
		});
		return res;
	}

	double computeSurface() const {
		if (Vec::dimension == 3) {
			double res = 0.0; // first = surface, second = volume;
			double faceArea = pow(1.0 / cellSize, 2);
			for (auto &i : um) {
				res += (6.0 - static_cast<double>(getNbNeighbours(i.first))) * faceArea;
			}
			return res;
		}
		return pow(1.0 / cellSize, 2) * static_cast<double>(um.size());
	}

	double getVolume() const {
		if (Vec::dimension == 3) return pow(1.0 / cellSize, 3) * static_cast<double>(um.size());
		return 0.0;
	}

	double computeSphericity() const {
		return (cbrt(M_PI) * (pow(6.0 * getVolume(), (2.0 / 3.0)))) / computeSurface();
	}

	// nb of occupied neighbour grid cells
	int getNbNeighbours(const Vec &cell) const {
		int res = 0;
		if (um.count(cell - Vec(0, 0, 1))) ++res;
		if (um.count(cell - Vec(0, 1, 0))) ++res;
		if (um.count(cell - Vec(1, 0, 0))) ++res;
		if (um.count(cell + Vec(0, 0, 1))) ++res;
		if (um.count(cell + Vec(0, 1, 0))) ++res;
		if (um.count(cell + Vec(1, 0, 0))) ++res;
		return res;
	}

	void clear() { um.clear(); }
};
}
#endif
