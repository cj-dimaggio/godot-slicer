#ifndef SLICER_H
#define SLICER_H

#include "scene/3d/spatial.h"
#include "sliced_mesh.h"

class Slicer : public Spatial {
    GDCLASS(Slicer, Spatial);

protected:
    static void _bind_methods();

public:
    Ref<SlicedMesh> slice(const Ref<Mesh> mesh, const Plane plane, const Ref<Material> cross_section_material);
    Slicer();
};

#endif // SLICER_H
