#include "intersector.h"

namespace Intersector {
    /**
     * FaceIntersectInfo is one place where we, superficially, deviate from the original
     * Ezy-Slice implementation. The idea is to generalize the plane-face relationship as
     * much as possible to try to reduce long series of conditionals with largely similar
     * blocks for code and hopefully make the actual process that's happening a bit easier
     * to grok.
    */
    struct FaceIntersectInfo {
        SideOfPlane sides[3];

        int num_of_points_above;
        Vector3 points_above[3];

        int num_of_points_below;
        Vector3 points_below[3];

        int num_of_points_on;
        Vector3 points_on[3];

        FaceIntersectInfo(const Plane &plane, const SlicerFace &face) {
            num_of_points_above = 0;
            num_of_points_below = 0;
            num_of_points_on = 0;

            SideOfPlane sides_of[3] = {
                get_side_of(plane, face.vertex[0]),
                get_side_of(plane, face.vertex[1]),
                get_side_of(plane, face.vertex[2])
            };

            for (int i = 0; i < 3; i++) {
                SideOfPlane side = sides_of[i];
                sides[i] = side;

                if (side == SideOfPlane::OVER)
                    points_above[num_of_points_above++] = face.vertex[i];
                else if (side == SideOfPlane::UNDER)
                    points_below[num_of_points_below++] = face.vertex[i];
                else
                    points_on[num_of_points_on++] = face.vertex[i];
            }
        }
    };

    // Similar to Face3::get_side_of but focused on a single point
    // rather than an entire face. Plane has a `is_point_over` method but
    // this doesn't give us enough information to know if the point is
    // actually laying on the plane without having to do an additional
    // calculation in Plane::has_point. The logic is so straightforward
    // I don't think we need to feel too bad about reimplementing to meet
    // our exact needs
    SideOfPlane get_side_of(const Plane &plane, Vector3 point) {
        real_t dist = plane.distance_to(point);
        if (dist > CMP_EPSILON) {
            return SideOfPlane::OVER;
        }

        if (dist < -CMP_EPSILON) {
            return SideOfPlane::UNDER;
        }

        return SideOfPlane::ON;
    }

    bool line_intersects(const Plane &plane, const Vector3 a, const Vector3 b, Vector3 &out) {
        Vector3 ab = b - a;
        real_t t = (plane.d - plane.normal.dot(a)) / (plane.normal.dot(ab));

        if (t >= CMP_EPSILON && t <= (1 + CMP_EPSILON)) {
            Vector3 intersect_point = a + t * ab;
            out.x = intersect_point.x;
            out.y = intersect_point.y;
            out.z = intersect_point.z;

            return true;
        }
        return false;
    }

    bool points_all_on_same_side(const SlicerFace &face, FaceIntersectInfo &info, SplitResult &result) {
        // This is actually a bit of a divergence from Ezy-Slice, where instead they just return and then handle
        // this case in a different loop. With the way we have things setup though I think we can just handle them
        // while we're here with all of already deduced info
        if (info.num_of_points_above == 3) {
            result.upper_faces.push_back(face);
            return true;
        } else if (info.num_of_points_below == 3) {
            result.lower_faces.push_back(face);
            return true;
        } else if (info.num_of_points_on == 3) {
            result.intersection_points.push_back(face.vertex[0]);
            result.intersection_points.push_back(face.vertex[1]);
            result.intersection_points.push_back(face.vertex[2]);
            return true;
        }

        return false;
    }

    bool one_side_is_parallel(const SlicerFace &face, FaceIntersectInfo &info, SplitResult &result) {
        // if two points are actually lying *on* the plane then we know there won't be any real intersection,
        // we can just reuse the facd as is after determining if the remaining point is above or below the plane
        if (info.num_of_points_on == 2) {
            if (info.num_of_points_above == 1) {
                result.upper_faces.push_back(face);
            } else {
                result.lower_faces.push_back(face);
            }
            return true;
        }

        return false;
    }

    bool pointed_away(const SlicerFace &face, FaceIntersectInfo &info, SplitResult &result) {
        // Similar to one_side_is_parallel except in this case only one point is on the plane
        // and the other 2 are on the same side
        if (info.num_of_points_on == 1) {
            if (info.num_of_points_above == 2) {
                result.upper_faces.push_back(face);
                return true;
            } else if (info.num_of_points_below == 2) {
                result.lower_faces.push_back(face);
                return true;
            }
        }

        return false;
    }

    bool face_split_in_half(const Plane &plane, const SlicerFace &face, FaceIntersectInfo &info, SplitResult &result) {
        // If one point is lying on the plane and the other 2 points are on either side all we really need to do is split
        // the triangle in half (or, more accurately, in two)
        if (info.num_of_points_on == 1) {
            Vector3 on = info.points_on[0];

            // We know that there is one on either side or else it would have been caught in `pointed_away`
            ERR_FAIL_COND_V(info.num_of_points_above != 1 || info.num_of_points_below != 1, false);
            Vector3 above = info.points_above[0];
            Vector3 below = info.points_below[0];

            Vector3 a = face.vertex[0];
            Vector3 b = face.vertex[1];
            Vector3 c = face.vertex[2];

            Vector3 intersect_point;
            if (!line_intersects(plane, above, below, intersect_point)) {
                ERR_FAIL_V(false);
            }

            result.intersection_points.push_back(on);
            result.intersection_points.push_back(intersect_point);

            SlicerFace upper_face;
            SlicerFace lower_face;

            // We need to make sure, for any new triangle we're generating, that the points are created clockwise so that
            // the face renders correctly. Sadly our FaceIntersectInfo helper fails us here and we need to fall back on
            // tedious conditionals to manually handle this logic. I'd really love a way of reliably generalizing this
            if (on == a) {
                upper_face = face.sub_face(a, b, intersect_point);
                lower_face = face.sub_face(a, intersect_point, c);
            } else if (on == b) {
                upper_face = face.sub_face(b, c, intersect_point);
                lower_face = face.sub_face(b, intersect_point, a);
            } else {
                upper_face = face.sub_face(c, a, intersect_point);
                lower_face = face.sub_face(c, intersect_point, b);
            }

            result.upper_faces.push_back(upper_face);
            result.lower_faces.push_back(lower_face);

            return true;
        }

        return false;
    }

    void full_split(const Plane &plane, const SlicerFace &face, FaceIntersectInfo &info, SplitResult &result) {
        // at this point, all edge cases have been tested and failed, we need to perform
        // full intersection tests against the lines. From this point onwards we will generate
        // 3 triangles
        ERR_FAIL_COND(info.num_of_points_on != 0);
        ERR_FAIL_COND(info.num_of_points_above == 0);
        ERR_FAIL_COND(info.num_of_points_below == 0);

        Vector3 a = face.vertex[0];
        Vector3 b = face.vertex[1];
        Vector3 c = face.vertex[2];

        // If we've gotten to this point then we can be able to confidently say that two points lie on one side
        // and one point lies on the other. We just need to find out which is which;
        Vector3 on_same_side_1;
        Vector3 on_same_side_2;
        Vector3 on_lone_side;

        if (info.num_of_points_above == 2) {
            on_same_side_1 = info.points_above[0];
            on_same_side_2 = info.points_above[1];
            on_lone_side = info.points_below[0];
        } else if (info.num_of_points_below == 2) {
            on_same_side_1 = info.points_below[0];
            on_same_side_2 = info.points_below[1];
            on_lone_side = info.points_above[0];
        } else {
            ERR_FAIL_MSG("Slicer's full_split method was called with unexpected intersection info");
        }

        Vector3 intersection_point_1;
        Vector3 intersection_point_2;
        if (!line_intersects(plane, on_same_side_1, on_lone_side, intersection_point_1) ||
            !line_intersects(plane, on_same_side_2, on_lone_side, intersection_point_2)) {
            ERR_FAIL();
        }

        SlicerFace lone_tri;
        SlicerFace same_tri_1;
        SlicerFace same_tri_2;

        // As mentioned in face_split_in_half, we need to make sure that we add our points
        // clockwise or else it won't render correctly. I'd love some way of generalizing this
        // to be less redundent
        if (on_lone_side == a) {
            lone_tri = face.sub_face(a, intersection_point_1, intersection_point_2);
            same_tri_1 = face.sub_face(b, intersection_point_2, intersection_point_1);
            same_tri_2 = face.sub_face(c, intersection_point_2, b);
        } else if (on_lone_side == b) {
            lone_tri = face.sub_face(b, intersection_point_2, intersection_point_1);
            same_tri_1 = face.sub_face(c, intersection_point_1, intersection_point_2);
            same_tri_2 = face.sub_face(a, intersection_point_1, c);
        } else {
            lone_tri = face.sub_face(c, intersection_point_1, intersection_point_2);
            same_tri_1 = face.sub_face(a, intersection_point_2, intersection_point_1);
            same_tri_2 = face.sub_face(b, intersection_point_2, a);
        }

        if (info.num_of_points_above == 2) {
            result.upper_faces.push_back(same_tri_1);
            result.upper_faces.push_back(same_tri_2);
            result.lower_faces.push_back(lone_tri);
        } else {
            result.lower_faces.push_back(same_tri_1);
            result.lower_faces.push_back(same_tri_2);
            result.upper_faces.push_back(lone_tri);

        }

        result.intersection_points.push_back(intersection_point_1);
        result.intersection_points.push_back(intersection_point_2);
    }

    // Face3 has its own split_by_plane but we need to make a few modifications to support
    // all the data that SlicerFace is responsible for holding. Also Ezy-Slice uses a few clever
    // tricks to handle edge cases
    //
    // Having result passed in and filled out by reference should hopefully allow us to reuse
    // the same one over a series of faces
    void split_face_by_plane(const Plane &plane, const SlicerFace &face, SplitResult &result) {
        FaceIntersectInfo info(plane, face);

        if (points_all_on_same_side(face, info, result)) {
            return;
        }

        if (one_side_is_parallel(face, info, result)) {
            return;
        }

        if (pointed_away(face, info, result)) {
            return;
        }

        if (face_split_in_half(plane, face, info, result)) {
            return;
        }

        // We've tried all of our clever edge cases, time to do a full intersection test
        full_split(plane, face, info, result);
    }
}
