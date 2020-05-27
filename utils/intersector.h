#ifndef INTERSECTOR_H
#define INTERSECTOR_H

#include "slicer_face.h"

namespace Intersector {
    // Note that this is slightly different than Face3::Side
    // as it refers to the position of a single Vector3 rather
    // than a face
    enum SideOfPlane {
        OVER,
        UNDER,
        ON,
    };

    struct SplitResult {
        Ref<Material> material;
        PoolVector<SlicerFace> upper_faces;
        PoolVector<SlicerFace> lower_faces;
        PoolVector<Vector3> intersection_points;
        bool was_split;

        void reset() {
            upper_faces.resize(0);
            lower_faces.resize(0);
            intersection_points.resize(0);
            was_split = false;
        }

        SplitResult() {
            was_split = false;
        }
    };

    // Similar, in spirit, to Face3::get_side_of but focused on a single
    // point rather than a face. Plane has a `is_point_over` method but
    // this doesn't give us enough information to know if the point is
    // actually laying on the plane without having to do an additional
    // calculation in Plane::has_point
    SideOfPlane get_side_of(const Plane &plane, Vector3 point);

    // Face3 has its own split_by_plane but we want to make a few custom modifications
    void split_face_by_plane(const Plane &plane, const SlicerFace &face, SplitResult &result);
} // Intersector


#endif // INTERSECTOR_H
