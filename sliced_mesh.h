#ifndef SLICED_MESH_H
#define SLICED_MESH_H

#include "core/resource.h"
#include "scene/resources/mesh.h"
#include "utils/intersector.h"

/**
 * A simple container for the results of a mesh slice.
 * upper_mesh contains the part of the mesh that was above
 * the plane normal and lower_mesh contains the part that was
 * below
*/
class SlicedMesh : public Resource {
    GDCLASS(SlicedMesh, Resource);

protected:
    static void _bind_methods();

public:
    Ref<Mesh> upper_mesh;
    Ref<Mesh> lower_mesh;

	void set_upper_mesh(const Ref<Mesh> &_upper_mesh) {
        upper_mesh = _upper_mesh;
    }
	Ref<Mesh> get_upper_mesh() const {
        return upper_mesh;
    };

	void set_lower_mesh(const Ref<Mesh> &_lower_mesh) {
        lower_mesh = _lower_mesh;
    }
	Ref<Mesh> get_lower_mesh() const {
        return lower_mesh;
    };

    SlicedMesh(Ref<Mesh> _upper_mesh, Ref<Mesh> _lower_mesh) {
        upper_mesh = _upper_mesh;
        lower_mesh = _lower_mesh;
    }

    /**
     * Transforms a vector of split results and a vector of faces representing
     * the cross section of a slice and creates an upper and lower mesh from them
    */
    SlicedMesh(const PoolVector<Intersector::SplitResult> &surface_splits, const PoolVector<SlicerFace> &cross_section_faces, Ref<Material> cross_section_material);
};

#endif // SLICED_MESH_H
