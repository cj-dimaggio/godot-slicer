#include "intersector.h"

namespace Intersector {
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
        // TODO - Updating split result int this case is a divergence from what EZYSlicer does. Does it work?
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
        // detect cases where two points lay straight on the plane, meaning
        // that the plane is actually parallel with one of the edges of the triangle
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
        // detect cases where one point is on the plane and the other two are on the same side
        if (info.num_of_points_on == 1) {
            if (info.num_of_points_above == 2) {
                result.upper_faces.push_back(face);
            } else if (info.num_of_points_below == 2) {
                result.lower_faces.push_back(face);
            }

            return true;
        }

        return false;
    }

    bool face_split_in_half(const Plane &plane, const SlicerFace &face, FaceIntersectInfo &info, SplitResult &result) {
        // check the cases where the points of the triangle actually lie on the plane itself
        // in these cases, there is only going to be 2 triangles, one for the upper HULL and
        // the other on the lower HULL
        if (info.num_of_points_on == 1) {
            Vector3 on = info.points_on[0];
            // We know that there is one on different sides or else it would have been caught in `pointed_away`
            ERR_FAIL_COND_V(info.num_of_points_above == 1 && info.num_of_points_below == 1, false);
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

            // We need to make sure we're placing our points clockwise (There's gotta be a better way to generalize this)
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
        // and one point lies on the other. We just need to fill out the values below correctly;
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

        // We need to make sure we're placing our points clockwise (There's gotta be a better way to generalize this)
        if (on_lone_side == a) {
            lone_tri = face.sub_face(a, intersection_point_1,intersection_point_2);
            same_tri_1 = face.sub_face(b, intersection_point_2, intersection_point_1);
            same_tri_2 = face.sub_face(c, intersection_point_2, b);
        } else if (on_lone_side == b) {
            lone_tri = face.sub_face(b, intersection_point_2, intersection_point_1);
            same_tri_1 = face.sub_face(c, intersection_point_1, intersection_point_2);
            same_tri_2 = face.sub_face(a, intersection_point_1, c);
        } else {
            lone_tri = face.sub_face(c, intersection_point_1,intersection_point_2);
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

    void split_face_by_plane(const Plane &plane, const SlicerFace &face, SplitResult &result) {
        FaceIntersectInfo info(plane, face);

        // we cannot intersect if the face's points all fall on the same side
        // of the plane.
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

        full_split(plane, face, info, result);
    }
}
