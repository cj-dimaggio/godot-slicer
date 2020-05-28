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

    /**
     * Calculates which side of the passed in plane the given point falls on
    */
    SideOfPlane get_side_of(const Plane &plane, Vector3 point);

    /**
     * Performs an intersection on the given face using the passed in plane and stores
     * the result in the result param.
    */
   // TODO - We followed Ezy-Slice's lead with making result a passed in a reference we modify.
   // instead of simply returning a result, ostensibly for performance reasons. But now that most
   // of the code has been written I think, in our implementation, all we're doing is making a new
   // SplitResult before calling this function and then throwing it into a vector rather than reusing
   // it; making this reference passing method completely unnecessary and confusing. Let's just update
   // it to return a SplitResult like you'd expect.
    void split_face_by_plane(const Plane &plane, const SlicerFace &face, SplitResult &result);
} // Intersector


#endif // INTERSECTOR_H
