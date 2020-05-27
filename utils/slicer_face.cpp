#include "slicer_face.h"
#include "face_filler.h"
#include "triangulator.h"

// Similar to Unity's https://docs.unity3d.com/ScriptReference/Vector3.OrthoNormalize.html
// Taken from somewhat from https://www.gamedev.net/forums/topic/585184-orthonormalize-two-vectors/
// There is also an example of Gram-Schmidt in godot's Basis::orthonormalize which does roughly
// the same thing
void ortho_normalize(Vector3 &normal, Vector3 &tangent) {
    normal.normalize();
    tangent -= normal * tangent.dot(normal);
    tangent.normalize();
}

PoolVector<SlicerFace> parse_mesh_arrays(const Mesh &mesh, int surface_idx, bool is_index_array) {
    PoolVector<SlicerFace> faces;
    int vert_count = is_index_array ? mesh.surface_get_array_index_len(surface_idx) : mesh.surface_get_array_len(surface_idx);
    if (vert_count == 0 || vert_count % 3 != 0) {
        return faces;
    }

    faces.resize(vert_count / 3);

    Array arrays = mesh.surface_get_arrays(surface_idx);
    FaceFiller filler(faces, arrays);

    if (is_index_array) {
        PoolVector<int> indices = arrays[Mesh::ARRAY_INDEX];
        auto indices_reader = indices.read();

        for (int i = 0; i < vert_count; i++) {
            filler.fill(i, indices_reader[i]);
        }
    } else {
        for (int i = 0; i < vert_count; i++) {
            filler.fill(i, i);
        }
    }

    return faces;
}

PoolVector<SlicerFace> SlicerFace::faces_from_surface(const Mesh &mesh, int surface_idx) {
    // We can really only handle meshes that are made up of triangles
    if (mesh.surface_get_primitive_type(surface_idx) != Mesh::PRIMITIVE_TRIANGLES) {
        return PoolVector<SlicerFace>();
    }

    if (mesh.surface_get_format(surface_idx) & Mesh::ARRAY_FORMAT_INDEX) {
        return parse_mesh_arrays(mesh, surface_idx, true);
    } else {
        return parse_mesh_arrays(mesh, surface_idx, false);
    }
}

SlicerFace SlicerFace::sub_face(Vector3 a, Vector3 b, Vector3 c) const {
    SlicerFace new_face(a, b, c);

    for (int i = 0; i < 3; i++) {
        Vector3 point = new_face.vertex[i];
        Vector3 bary = barycentric_weights(point);

        if (has_normals) {
            new_face.has_normals = true;
            new_face.normal[i] = (normal[0] * bary[0]) + (normal[1] * bary[1]) + (normal[2] * bary[2]);
        }

        if (has_colors) {
            new_face.has_colors = true;
            new_face.color[i] = (color[0] * bary[0]) + (color[1] * bary[1]) + (color[2] * bary[2]);
        }

        if (has_uvs) {
            new_face.has_uvs = true;
            new_face.uv[i] = (uv[0] * bary[0]) + (uv[1] * bary[1]) + (uv[2] * bary[2]);
        }

        if (has_uv2s) {
            new_face.has_uv2s = true;
            new_face.uv2[i] = (uv2[0] * bary[0]) + (uv2[1] * bary[1]) + (uv2[2] * bary[2]);
        }

        if (has_tangents) {
            new_face.has_tangents = true;
            new_face.tangent[i] = (tangent[0] * bary[0]) + (tangent[1] * bary[1]) + (tangent[2] * bary[2]);
        }

        if (has_bones) {
            new_face.has_bones = true;
            new_face.bones[i] = (bones[0] * bary[0]) + (bones[1] * bary[1]) + (bones[2] * bary[2]);
        }

        if (has_weights) {
            new_face.has_weights = true;
            new_face.weights[i] = (weights[0] * bary[0]) + (weights[1] * bary[1]) + (weights[2] * bary[2]);
        }
    }

    return new_face;
}

/**
 * Compute and set the tangents of this triangle
 * Derived From https://answers.unity.com/questions/7789/calculating-tangents-vector4.html
 * Taken almost verbatim from EZYSlice
 */
void SlicerFace::compute_tangents() {
    // computing tangents requires both UV and normals set
    if (!has_normals || !has_uvs) {
        return;
    }

    real_t x1 = vertex[1].x - vertex[0].x;
    real_t x2 = vertex[2].x - vertex[0].x;
    real_t y1 = vertex[1].y - vertex[0].y;
    real_t y2 = vertex[2].y - vertex[0].y;
    real_t z1 = vertex[1].z - vertex[0].z;
    real_t z2 = vertex[2].z - vertex[0].z;

    real_t s1 = uv[1].x - uv[0].x;
    real_t s2 = uv[2].x - uv[0].x;
    real_t t1 = uv[1].y - uv[0].y;
    real_t t2 = uv[2].y - uv[0].y;

    real_t r = 1.0f / (s1 * t2 - s2 * t1);

    Vector3 sdir = Vector3((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
    Vector3 tdir = Vector3((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

    // This can definitely be dried up a bit
    Vector3 n1 = normal[0];
    Vector3 nt1 = sdir;
    ortho_normalize(n1, nt1);
    SlicerVector4 tan_a = SlicerVector4(nt1.x, nt1.y, nt1.z, (n1.cross(nt1).dot(tdir) < 0.0f) ? -1.0f : 1.0f);

    Vector3 n2 = normal[1];
    Vector3 nt2 = sdir;
    ortho_normalize(n2, nt2);
    SlicerVector4 tan_b = SlicerVector4(nt2.x, nt2.y, nt2.z, (n2.cross(nt2).dot(tdir) < 0.0f) ? -1.0f : 1.0f);

    Vector3 n3 = normal[2];
    Vector3 nt3 = sdir;
    ortho_normalize(n3, nt3);
    SlicerVector4 tan_c = SlicerVector4(nt3.x, nt3.y, nt3.z, (n3.cross(nt3).dot(tdir) < 0.0f) ? -1.0f : 1.0f);

    set_tangents(tan_a, tan_b, tan_c);
}

/**
 * Calculate the Barycentric coordinate weight values u-v-w for Point p in respect to the provided
 * triangle. This is useful for computing new UV coordinates for arbitrary points.
 */
Vector3 SlicerFace::barycentric_weights(Vector3 p) const {
    Vector3 a = vertex[0];
    Vector3 b = vertex[1];
    Vector3 c = vertex[2];

    Vector3 m = (b - a).cross(c - a);

    real_t nu;
    real_t nv;
    real_t ood;

    real_t x = Math::abs(m.x);
    real_t y = Math::abs(m.y);
    real_t z = Math::abs(m.z);

    // compute areas of plane with largest projections
    if (x >= y && x >= z) {
        // area of PBC in yz plane
        nu = Triangulator::tri_area_2d(p.y, p.z, b.y, b.z, c.y, c.z);
        // area of PCA in yz plane
        nv = Triangulator::tri_area_2d(p.y, p.z, c.y, c.z, a.y, a.z);
        // 1/2*area of ABC in yz plane
        ood = 1.0f / m.x;
    } else if (y >= x && y >= z) {
        // project in xz plane
        nu = Triangulator::tri_area_2d(p.x, p.z, b.x, b.z, c.x, c.z);
        nv = Triangulator::tri_area_2d(p.x, p.z, c.x, c.z, a.x, a.z);
        ood = 1.0f / -m.y;
    } else {
        // project in xy plane
        nu = Triangulator::tri_area_2d(p.x, p.y, b.x, b.y, c.x, c.y);
        nv = Triangulator::tri_area_2d(p.x, p.y, c.x, c.y, a.x, a.y);
        ood = 1.0f / m.z;
    }

    real_t u = nu * ood;
    real_t v = nv * ood;
    real_t w = 1.0f - u - v;

    return Vector3(u, v, w);
}
