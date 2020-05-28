#ifndef INTERSECTOR_H
#define INTERSECTOR_H

#include "slicer_face.h"

/**
 * Contains functions related to finding intersection points
 * on SlicerFaces
*/
namespace Intersector {
    // Note that this is slightly different than Face3::Side,
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

        void reset() {
            upper_faces.resize(0);
            lower_faces.resize(0);
            intersection_points.resize(0);
        }

        SplitResult() {}
    };

    /**
     * Calculates which side of the passed in plane the given point falls on
    */
    SideOfPlane get_side_of(const Plane &plane, Vector3 point);

    /**
     * Performs an intersection on the given face using the passed in plane and stores
     * the result in the result param.
    */
    void split_face_by_plane(const Plane &plane, const SlicerFace &face, SplitResult &result);
} // Intersector


#endif // INTERSECTOR_H
