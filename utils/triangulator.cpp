#include "triangulator.h"
#include <limits>
#include <algorithm>

/**
 * Represents a 3D Vertex which has been mapped onto a 2D surface
 * and is mainly used in MonotoneChain to triangulate a set of vertices
 * against a flat plane.
 */
struct Mapped2D {
    Vector3 original;
    Vector2 mapped;

    Mapped2D() {}

    Mapped2D(Vector3 newOriginal, Vector3 u, Vector3 v) {
        original = newOriginal;
        mapped = Vector2(newOriginal.dot(u), newOriginal.dot(v));
    }

    struct Comparator {
        _FORCE_INLINE_ bool operator()(const Mapped2D &a, const Mapped2D &b) const {
            Vector2 x = a.mapped;
            Vector2 p = b.mapped;

            if (x.x != p.x) {
                return x.x < p.x;
            } else {
                 return x.y < p.y;
            }
        }
    };
};

namespace Triangulator {
    real_t tri_area_2d(real_t x1, real_t y1, real_t x2, real_t y2, real_t x3, real_t y3) {
        return (x1 - x2) * (y2 - y3) - (x2 - x3) * (y1 - y2);
    }
    
    // Godot has a QuickHull function (along with VHACD bindings which I'm sure has all kind of crazy smart stuff in it)
    // But as this is primarily a learning exercise (and because monotone chain has a slightly different time complexity
    // along with the need to support uv mappings and such) let's try to implement this ourselves
    PoolVector<SlicerFace> monotone_chain(const PoolVector<Vector3> &interception_points, Vector3 plane_normal) {
        // So we'll be using the monotone_chain algorithm to try to get a convex hull from our assortment of
        // interception_points along our plane

        int count = interception_points.size();
        PoolVector<SlicerFace> result;

        if (count < 3) {
            return result;
        }

        // first, we map from 3D points into a 2D plane represented by the provided normal
        Vector3 u = plane_normal.cross(Vector3( 0, 1, 0 )).normalized();
        if (u == Vector3(0, 0, 0)) {
            u = plane_normal.cross(Vector3(0, 0, -1)).normalized();
        }
        Vector3 v = u.cross(plane_normal);

        // generate an array of mapped values
        Vector<Mapped2D> mapped;
        mapped.resize(count);

        // these values will be used to generate new UV coordinates later on
        real_t max_div_x = std::numeric_limits<real_t>::min() ;
        real_t max_div_y = std::numeric_limits<real_t>::min() ;
        real_t min_div_x = std::numeric_limits<real_t>::max() ;
        real_t min_div_y = std::numeric_limits<real_t>::max() ;

        // map the 3D vertices into the 2D mapped values
        for (int i = 0; i < count; i++) {
            Vector3 vert_to_add = interception_points[i];
            Mapped2D new_mapped_value = Mapped2D(vert_to_add, u, v);
            Vector2 map_val = new_mapped_value.mapped;

            // grab our maximal values so we can map UV's in a proper range
            max_div_x = std::max(max_div_x, map_val.x);
            max_div_y = std::max(max_div_y, map_val.y);
            min_div_x = std::min(min_div_x, map_val.x);
            min_div_y = std::min(min_div_y, map_val.y);

            mapped.set(i, new_mapped_value);
        }

        // sort our newly generated array values
        mapped.sort_custom<Mapped2D::Comparator>();

        // our final hull mappings will end up in here
        PoolVector<Mapped2D> hulls;
        hulls.resize(count + 1);
        PoolVector<Mapped2D>::Write hulls_writer = hulls.write();

        int k = 0;

        // build the lower hull of the chain
        for (int i = 0; i < count; i++) {
            while (k >= 2) {
                Vector2 mA = hulls[k - 2].mapped;
                Vector2 mB = hulls[k - 1].mapped;
                Vector2 mC = mapped[i].mapped;

                if (tri_area_2d(mA.x, mA.y, mB.x, mB.y, mC.x, mC.y) > 0.0f) {
                    break;
                }

                k--;
            }

            hulls_writer[k++] = mapped[i];
        }

        // build the upper hull of the chain
        for (int i = count - 2, t = k + 1; i >= 0; i--) {
            while (k >= t) {
                Vector2 mA = hulls[k - 2].mapped;
                Vector2 mB = hulls[k - 1].mapped;
                Vector2 mC = mapped[i].mapped;

                if (tri_area_2d(mA.x, mA.y, mB.x, mB.y, mC.x, mC.y) > 0.0f) {
                    break;
                }

                k--;
            }

            hulls_writer[k++] = mapped[i];
        }

        // finally we can build our mesh, generate all the variables
        // and fill them up
        int vert_count = k - 1;
        int tri_count = (vert_count - 2) * 3;

        // this should not happen, but here just in case
        if (vert_count < 3) {
            return result;
        }

        // ensure List does not dynamically grow, performing copy ops each time!
        result.resize(tri_count / 3);
        PoolVector<SlicerFace>::Write result_writer = result.write();

        float width = max_div_x - min_div_x;
        float height = max_div_y - min_div_y;

        int index_count = 1;

        // generate both the vertices and uv's in this loop
        for (int i = 0; i < tri_count; i += 3) {
            // the Vertices in our triangle
            Mapped2D pos_a = hulls[0];
            Mapped2D pos_b = hulls[index_count];
            Mapped2D pos_c = hulls[index_count + 1];

            // generate UV Maps
            Vector2 uv_a = pos_a.mapped;
            Vector2 uv_b = pos_b.mapped;
            Vector2 uv_c = pos_c.mapped;

            uv_a.x = (uv_a.x - min_div_x) / width;
            uv_a.y = (uv_a.y - min_div_y) / height;

            uv_b.x = (uv_b.x - min_div_x) / width;
            uv_b.y = (uv_b.y - min_div_y) / height;

            uv_c.x = (uv_c.x - min_div_x) / width;
            uv_c.y = (uv_c.y - min_div_y) / height;

            SlicerFace new_face = SlicerFace(pos_a.original, pos_b.original, pos_c.original);

            // TODO - Add support for texture regions
            new_face.set_uvs(uv_a, uv_b, uv_c);

            // the normals is the same for all vertices since the final mesh is completely flat
            new_face.set_normals(plane_normal, plane_normal, plane_normal);
            new_face.compute_tangents();

            result_writer[i / 3] = new_face;

            index_count++;
        }
        result_writer.release();

        return result;
    }
}
