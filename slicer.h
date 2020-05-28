#ifndef SLICER_H
#define SLICER_H

#include "scene/3d/spatial.h"
#include "scene/3d/mesh_instance.h"
#include "sliced_mesh.h"

/**
 * Helper for cutting a convex mesh along a plane and returning
 * two new meshes representing both sides of the cut
*/
class Slicer : public Spatial {
    GDCLASS(Slicer, Spatial);

protected:
    static void _bind_methods();

public:
    /**
     * Slice the passed in mesh along the passed in plane, setting the interrior cut surface to the passed in material
    */
    Ref<SlicedMesh> slice_by_plane(const Ref<Mesh> mesh, const Plane plane, const Ref<Material> cross_section_material);

    /**
     * Generates a plane based on the given position and normal and perform a cut along that plane
    */
    Ref<SlicedMesh> slice_mesh(const Ref<Mesh> mesh, const Vector3 position, const Vector3 normal, const Ref<Material> cross_section_material);

    /**
     * Generates a plane based on the given position and normal and offsets it by the given Transform before applying the slice
    */
    Ref<SlicedMesh> slice(const Ref<Mesh> mesh, const Transform mesh_transform, const Vector3 position, const Vector3 normal, const Ref<Material> cross_section_material);
    Slicer() {};
};

#endif // SLICER_H
