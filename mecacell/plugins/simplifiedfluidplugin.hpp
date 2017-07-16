#ifndef MECACELL_SIMPLIFIED_FLUID_PLUGIN_HPP
#define MECACELL_SIMPLIFIED_FLUID_PLUGIN_HPP

namespace MecaCell {
/**
 * @brief simplified fluid drag plugin. Usefull to simulate swimming and/or flight in a
 * computation friendly manner. No turbulences, just force applied to the cells according
 * to their speed and the fluidDensity;
 * Possibility to specify a fluid velocity in order to simulate simple currents.
 *
 * THIS IS A PLUGIN. Instanciate where you want and register using World::registerPlugin
 *
 * @tparam C the cell type
 */
template <typename C> struct SimplifiedFluidPlugin {
	double fluidDensity = 1e-4;  /// bigger fluid density = more resistance
	double dragCoef = 1.0;  /// TODO: should be possible to access the dragCoef of each cell

	MecaCell::Vector3D fluidVelocity{0, 0, 0};
	// MecaCell::Vector3D();  /// to generate currents. TODO: various
	/// current patterns : constant, random,
	/// from a map...

	/**
	 * @brief HOOK: computes the forces to be applied to each cells. Called at every
	 * world update;
	 *
	 * @tparam W world type
	 * @param w the world
	 */
	template <typename W> void beginUpdate(W *w) {
		if (w->cells.size() > 0) {
			double avgCellRadius = 0.0;
			for (auto &c : w->cells) avgCellRadius += c->getBoundingBoxRadius();
			avgCellRadius /= static_cast<double>(w->cells.size());
			MecaCell::Grid<C *> grid(avgCellRadius * 1.5);
			std::unordered_map<size_t, std::vector<MecaCell::Vector3D>>
			    exposedVoxels;  // voxelId -> list of exposed normals

			// first we voxelize
			for (auto &c : w->cells) grid.insert(c);

			// then we find voxels that have at least one exposed face
			for (auto &u : grid.getUnorderedMap()) {
				if (!grid.getUnorderedMap().count(u.first + MecaCell::Vector3D(1, 0, 0)))
					exposedVoxels[u.second].push_back(MecaCell::Vector3D(1, 0, 0));
				if (!grid.getUnorderedMap().count(u.first + MecaCell::Vector3D(-1, 0, 0)))
					exposedVoxels[u.second].push_back(MecaCell::Vector3D(-1, 0, 0));
				if (!grid.getUnorderedMap().count(u.first + MecaCell::Vector3D(0, 1, 0)))
					exposedVoxels[u.second].push_back(MecaCell::Vector3D(0, 1, 0));
				if (!grid.getUnorderedMap().count(u.first + MecaCell::Vector3D(0, -1, 0)))
					exposedVoxels[u.second].push_back(MecaCell::Vector3D(0, -1, 0));
				if (!grid.getUnorderedMap().count(u.first + MecaCell::Vector3D(0, 0, 1)))
					exposedVoxels[u.second].push_back(MecaCell::Vector3D(0, 0, 1));
				if (!grid.getUnorderedMap().count(u.first + MecaCell::Vector3D(0, 0, -1)))
					exposedVoxels[u.second].push_back(MecaCell::Vector3D(0, 0, -1));
			}

			// we can now apply the correct forces to each exposed cell
			for (auto &e : exposedVoxels) {
				double area = std::pow(grid.getCellSize(), 2);
				// we need the nb of cells to divide forces when a voxel contains multiple cells
				// (to address overlaps) :
				// Cells at cell grid grid.getOrderedVec()[e.first].second
				double nbCells = grid.getOrderedVec()[e.first].second.size();
				for (auto &c : grid.getOrderedVec()[e.first].second) {  // c = exposed cell
					for (const auto &dir : e.second) {
						double normalSpeed = (c->getBody().getVelocity() - fluidVelocity).dot(dir);
						if (normalSpeed > 0) {
							auto F =
							    (-0.5 * dir * fluidDensity * dragCoef * area * normalSpeed) / nbCells;
							c->getBody().receiveForce(F);
							// c->setColorHSV(F.length() * 300.0, 0.8, 0.8);
						}
					}
				}
			}
		}
	}
};
}  // namespace MecaCell
#endif
