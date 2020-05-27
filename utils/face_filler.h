#ifndef FACE_FILLER_H
#define FACE_FILLER_H

#include "slicer_face.h"

// Mimic logic in TriangleMesh#Create
_FORCE_INLINE_ Vector3 snap_vertex(Vector3 v) {
    return v.snapped(Vector3(0.0001, 0.0001, 0.0001));
}

struct FaceFiller {
    PoolVector<SlicerFace>::Write faces_writer;
    PoolVector<Vector3>::Read vertices_reader;

    bool has_normals;
    PoolVector<Vector3>::Read normals_reader;

    bool has_tangents;
    PoolVector<real_t>::Read tangents_reader;

    bool has_colors;
    PoolVector<Color>::Read colors_reader;

    bool has_bones;
    PoolVector<real_t>::Read bones_reader;

    bool has_weights;
    PoolVector<real_t>::Read weights_reader;

    bool has_uvs;
    PoolVector<Vector2>::Read uvs_reader;

    bool has_uv2s;
    PoolVector<Vector2>::Read uv2s_reader;

    FaceFiller(PoolVector<SlicerFace> &faces, const Array &surface_arrays) {
        faces_writer = faces.write();

        PoolVector<Vector3> vertices = surface_arrays[Mesh::ARRAY_VERTEX];
        vertices_reader = vertices.read();

        PoolVector<Vector3> normals = surface_arrays[Mesh::ARRAY_NORMAL];
        normals_reader = normals.read();
        has_normals = normals.size() > 0 && normals.size() == vertices.size();

        PoolVector<real_t> tangents = surface_arrays[Mesh::ARRAY_TANGENT];
        tangents_reader = tangents.read();
        has_tangents = tangents.size() > 0 && tangents.size() == vertices.size() * 4;
        
        PoolVector<Color> colors = surface_arrays[Mesh::ARRAY_COLOR];
        colors_reader = colors.read();
        has_colors = colors.size() > 0 && colors.size() == vertices.size();

        PoolVector<real_t> bones = surface_arrays[Mesh::ARRAY_BONES];
        bones_reader = bones.read();
        has_bones = bones.size() > 0 && bones.size() == vertices.size() * 4;

        PoolVector<real_t> weights = surface_arrays[Mesh::ARRAY_WEIGHTS];
        weights_reader = weights.read();
        has_weights = weights.size() > 0 && weights.size() == vertices.size() * 4;

        PoolVector<Vector2> uvs = surface_arrays[Mesh::ARRAY_TEX_UV];
        uvs_reader = uvs.read();
        has_uvs = uvs.size() > 0 && uvs.size() == vertices.size();

        PoolVector<Vector2> uv2s = surface_arrays[Mesh::ARRAY_TEX_UV2];
        uv2s_reader = uv2s.read();
        has_uv2s = uv2s.size() > 0 && uv2s.size() == vertices.size();
    }

    _FORCE_INLINE_ void fill(int set_idx, int lookup_idx) {
        // TODO - Can we rewrite this to work on more than one index at a time
        // and still be readable? Would we get a performance boost?
        int face_idx = set_idx / 3;
        int set_offset = set_idx % 3;

        if (set_offset == 0) {
            faces_writer[face_idx].has_normals = has_normals;
            faces_writer[face_idx].has_tangents = has_tangents;
            faces_writer[face_idx].has_colors = has_colors;
            faces_writer[face_idx].has_bones = has_bones;
            faces_writer[face_idx].has_weights = has_weights;
            faces_writer[face_idx].has_uvs = has_uvs;
            faces_writer[face_idx].has_uv2s = has_uv2s;
        }

        faces_writer[face_idx].vertex[set_offset] = snap_vertex(vertices_reader[lookup_idx]);

        if (has_normals) {
            faces_writer[face_idx].normal[set_offset] = normals_reader[lookup_idx];
        }

        if (has_tangents) {
            faces_writer[face_idx].tangent[set_offset] = SlicerVector4(
                tangents_reader[lookup_idx * 4],
                tangents_reader[lookup_idx * 4 + 1],
                tangents_reader[lookup_idx * 4 + 2],
                tangents_reader[lookup_idx * 4 + 3]
            );
        }

        if (has_colors) {
            faces_writer[face_idx].color[set_offset] = colors_reader[lookup_idx];
        }

        if (has_bones) {
            faces_writer[face_idx].bones[set_offset] = SlicerVector4(
                bones_reader[lookup_idx * 4],
                bones_reader[lookup_idx * 4 + 1],
                bones_reader[lookup_idx * 4 + 2],
                bones_reader[lookup_idx * 4 + 3]
            );
        }

        if (has_weights) {
            faces_writer[face_idx].weights[set_offset] = SlicerVector4(
                weights_reader[lookup_idx],
                weights_reader[lookup_idx * 4 + 1],
                weights_reader[lookup_idx * 4 + 2],
                weights_reader[lookup_idx * 4 + 3]
            );
        }

        if (has_uvs) {
            faces_writer[face_idx].uv[set_offset] = uvs_reader[lookup_idx];
        }

        if (has_uv2s) {
            faces_writer[face_idx].uv2[set_offset] = uv2s_reader[lookup_idx];
        }
    }

    ~FaceFiller() {
        faces_writer.release();
    }
};

#endif // FACE_FILLER_H
