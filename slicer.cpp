#include "slicer.h"
#include "utils/slicer_face.h"
#include "utils/intersector.h"
#include "utils/triangulator.h"

Ref<SlicedMesh> Slicer::slice_by_plane(const Ref<Mesh> mesh, const Plane plane, const Ref<Material> cross_section_material) {
    // TODO - This function is a little heavy. Maybe we should break it up
    if (mesh.is_null()) {
        return Ref<SlicedMesh>();
    }

    PoolVector<Intersector::SplitResult> split_results;
    split_results.resize(mesh->get_surface_count());
    PoolVector<Intersector::SplitResult>::Write split_results_writer = split_results.write();

    // The upper and lower meshes will share the same intersection points
    PoolVector<Vector3> intersection_points;

    for (int i = 0; i < mesh->get_surface_count(); i++) {
        Intersector::SplitResult results = split_results[i];

        results.material = mesh->surface_get_material(i);
        PoolVector<SlicerFace> faces = SlicerFace::faces_from_surface(**mesh, i);
        PoolVector<SlicerFace>::Read faces_reader = faces.read();

        for (int j = 0; j < faces.size(); j++) {
            Intersector::split_face_by_plane(plane, faces_reader[j], results);
        }

        int ip_size = intersection_points.size();
        intersection_points.resize(ip_size + results.intersection_points.size());
        PoolVector<Vector3>::Write ip_writer = intersection_points.write();
        for (int j = 0; j < results.intersection_points.size(); j ++) {
            ip_writer[ip_size + j] = results.intersection_points[j];
        }
        ip_writer.release();
        results.intersection_points.resize(0);

        split_results_writer[i] = results;
    }


    // If no intersection has occurred then there's really nothing for us to do
    // but still, is this the expected behavior? Would it be better to return an
    // actual SliceMesh with either the upper_mesh or lower_mesh null?
    if (intersection_points.size() == 0) {
        return Ref<SlicedMesh>();
    }

    PoolVector<SlicerFace> cross_section_faces = Triangulator::monotone_chain(intersection_points, plane.normal);

    SlicedMesh *sliced_mesh = memnew(SlicedMesh(split_results, cross_section_faces, cross_section_material));
    return Ref<SlicedMesh>(sliced_mesh);
}

Ref<SlicedMesh> Slicer::slice_mesh(const Ref<Mesh> mesh, const Vector3 position, const Vector3 normal, const Ref<Material> cross_section_material) {
    Plane plane(normal, normal.dot(position));
    return slice_by_plane(mesh, plane, cross_section_material);
}

Ref<SlicedMesh> Slicer::slice(const Ref<Mesh> mesh, const Transform mesh_transform, const Vector3 position, const Vector3 normal, const Ref<Material> cross_section_material) {
    // We need to reorient the plane so that it will correctly slice the mesh whose vertexes are based on the origin
    Vector3 origin = position - mesh_transform.origin;
    real_t dist = normal.dot(origin);
    Vector3 adjusted_normal = mesh_transform.basis.xform_inv(normal);

    return slice_by_plane(mesh, Plane(adjusted_normal, dist), cross_section_material);
}

void Slicer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("slice_by_plane", "mesh", "plane", "cross_section_material"), &Slicer::slice_by_plane, Variant::NIL);
    ClassDB::bind_method(D_METHOD("slice_mesh", "mesh", "position", "normal", "cross_section_material"), &Slicer::slice_mesh, Variant::NIL);
    ClassDB::bind_method(D_METHOD("slice", "mesh_instance", "mesh_transform", "position", "normal", "cross_section_material"), &Slicer::slice, Variant::NIL);
}
