#ifndef GRID_HPP
#define GRID_HPP

#include <vector>
#include <set>
#include <iostream>
#include <unordered_map>
#include "tools.h"
using namespace std;

namespace MecaCell {
template <typename O> class Grid {
private:
	double cellSize; // actually it's 1/cellSize, just so we can multiply
	unordered_map<Vec, vector<O>> um;

public:
	Grid(double cs) : cellSize(1.0 / cs) {}

	double getCellSize() const { return 1.0 / cellSize; }
	const unordered_map<Vec, vector<O>> &getContent() const { return um; }

	void insert(const O &obj) {
		Vec center = ptr(obj)->getPosition() * cellSize;
		double radius = ptr(obj)->getRadius() * cellSize;
		Vec minCorner = center - radius;
		Vec maxCorner = center + radius;
		minCorner.iterateTo(maxCorner, [&](Vec v) { um[v].push_back(obj); });
	}

	void insert(const O &obj, const Vec &p0, const Vec &p1,
	            const Vec &p2) { // insert triangles
		Vec blf(min(p0.x, min(p1.x, p2.x)), min(p0.y, min(p1.y, p2.y)),
		        min(p0.z, min(p1.z, p2.z)));
		Vec trb(max(p0.x, max(p1.x, p2.x)), max(p0.y, max(p1.y, p2.y)),
		        max(p0.z, max(p1.z, p2.z)));
		double cs = 1.0 / cellSize;
		getIndexFromPosition(blf).iterateTo(getIndexFromPosition(trb) + 1, [&](const Vec &v) {
			Vec center = cs * v;
			std::pair<bool, Vec> projec = projectionIntriangle(p0, p1, p2, center);
			if ((center - projec.second).sqlength() < 0.8 * cs * cs) {
				if (projec.first || closestDistToTriangleEdge(p0, p1, p2, center) < 0.87 * cs) {
					um[v].push_back(obj);
				}
			}
		});
	}

	Vec getIndexFromPosition(const Vec &v) {
		Vec res = v * cellSize;
		return Vec(floor(res.x), floor(res.y), floor(res.z));
	}

	set<O> retrieveUnique(const Vec &coord, double r) const {
		set<O> res;
		Vec center = coord * cellSize;
		double radius = r * cellSize;
		Vec minCorner = center - radius;
		Vec maxCorner = center + radius;
		// TODO check if faster with a set (uniques...) and by removing  selfcollision
		minCorner.iterateTo(maxCorner, [&](const Vec &v) {
			if (um.count(v)) {
				for (auto &e : um.at(v)) {
					res.insert(e);
				}
			}
		});
		return res;
	}

	vector<O> retrieve(const Vec &coord, double r) const {
		vector<O> res;
		Vec center = coord * cellSize;
		double radius = r * cellSize;
		Vec minCorner = center - radius;
		Vec maxCorner = center + radius;
		// TODO check if faster with a set (uniques...) and by removing  selfcollision
		minCorner.iterateTo(maxCorner, [&](const Vec &v) {
			if (um.count(v)) res.insert(res.end(), um.at(v).begin(), um.at(v).end());
		});
		return res;
	}

	vector<O> retrieve(const O &obj) const {
		vector<O> res;
		Vec center = ptr(obj)->getPosition() * cellSize;
		double radius = ptr(obj)->getRadius() * cellSize;
		Vec minCorner = center - radius;
		Vec maxCorner = center + radius;
		minCorner.iterateTo(maxCorner, [this, &res](const Vec &v) {
			if (um.count(v)) res.insert(res.end(), um.at(v).begin(), um.at(v).end());
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
		if (Vec::dimension == 3)
			return pow(1.0 / cellSize, 3) * static_cast<double>(um.size());
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
