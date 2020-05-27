#include "sliced_mesh.h"
#include "utils/surface_filler.h"

void create_surface(const PoolVector<SlicerFace> &faces, const Ref<Material> material, ArrayMesh &mesh) {
    if (faces.size() == 0) {
        return;
    }

    SurfaceFiller filler(faces);

    for (int i = 0; i < faces.size() * 3; i++) {
        filler.fill(i, i);
    }

    filler.add_to_mesh(mesh, material);
}

void create_cross_section_surface(const PoolVector<SlicerFace> &faces, const Ref<Material> material, ArrayMesh &mesh, bool is_upper) {
    if (faces.size() == 0) {
        return;
    }

    SurfaceFiller filler(faces);

    for (int i = 0; i < faces.size(); i++) {
        // add triangles in clockwise or counterclockwise
        // depending on direction so that it renders correctly
        if (is_upper) {
            filler.fill(i * 3, i * 3);
            filler.fill(i * 3 + 1, i * 3 + 2);
            filler.fill(i * 3 + 2, i * 3 + 1);
        } else {
            filler.fill(i * 3, i * 3);
            filler.fill(i * 3 + 1, i * 3 + 1);
            filler.fill(i * 3 + 2, i * 3 + 2);
        }
    }

    filler.add_to_mesh(mesh, material);
}

Mesh* create_mesh_half(
    const PoolVector<Intersector::SplitResult> &surface_splits,
    const PoolVector<SlicerFace> &cross_section_faces,
    Ref<Material> cross_section_material,
    bool is_upper
) {
    ArrayMesh *mesh = memnew(ArrayMesh);

    for (int i = 0; i < surface_splits.size(); i++) {
        if (is_upper) {
            create_surface(surface_splits[i].upper_faces, surface_splits[i].material, *mesh);
        } else {
            create_surface(surface_splits[i].lower_faces, surface_splits[i].material, *mesh);
        }
    }


    if (cross_section_material.is_null() && mesh->get_surface_count() > 0) {
        cross_section_material = mesh->surface_get_material(0);
    }

    create_cross_section_surface(cross_section_faces, cross_section_material, *mesh, is_upper);
    return mesh;
}

void SlicedMesh::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_upper_mesh", "mesh"), &SlicedMesh::set_upper_mesh);
    ClassDB::bind_method(D_METHOD("get_upper_mesh"), &SlicedMesh::get_upper_mesh);
    ClassDB::bind_method(D_METHOD("set_lower_mesh", "mesh"), &SlicedMesh::set_lower_mesh);
    ClassDB::bind_method(D_METHOD("get_lower_mesh"), &SlicedMesh::get_lower_mesh);

    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "upper_mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_upper_mesh", "get_upper_mesh");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "lower_mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_lower_mesh", "get_lower_mesh");
}

SlicedMesh::SlicedMesh(const PoolVector<Intersector::SplitResult> &surface_splits, const PoolVector<SlicerFace> &cross_section_faces, const Ref<Material> cross_section_material) {
    upper_mesh = Ref<Mesh>(create_mesh_half(surface_splits, cross_section_faces, cross_section_material, true));
    lower_mesh = Ref<Mesh>(create_mesh_half(surface_splits, cross_section_faces, cross_section_material, false));
}
