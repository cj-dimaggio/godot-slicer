#ifndef SURFACE_FILLER_H
#define SURFACE_FILLER_H

#include "slicer_face.h"

/**
 * The inverse of FaceFiller, this struct is responsible for taking
 * SlicerFaces and serializing them back into vertex arrays for Godot
 * to read into a mesh surface
*/
struct SurfaceFiller {
    bool has_normals;
    bool has_tangents;
    bool has_colors;
    bool has_bones;
    bool has_weights;
    bool has_uvs;
    bool has_uv2s;

    PoolVector<SlicerFace>::Read faces_reader;

    Array arrays;
    PoolVector<Vector3> vertexes;
    PoolVector<Vector3>::Write vertexes_writer;

    PoolVector<Vector3> normals;
    PoolVector<Vector3>::Write normals_writer;

    PoolVector<real_t> tangents;
    PoolVector<real_t>::Write tangents_writer;

    PoolVector<Color> colors;
    PoolVector<Color>::Write colors_writer;

    PoolVector<real_t> bones;
    PoolVector<real_t>::Write bones_writer;

    PoolVector<real_t> weights;
    PoolVector<real_t>::Write weights_writer;

    PoolVector<Vector2> uvs;
    PoolVector<Vector2>::Write uvs_writer;

    PoolVector<Vector2> uv2s;
    PoolVector<Vector2>::Write uv2s_writer;

    SurfaceFiller(const PoolVector<SlicerFace> &faces) {
        SlicerFace first_face = faces[0];

        has_normals = first_face.has_normals;
        has_tangents = first_face.has_tangents;
        has_colors = first_face.has_colors;
        has_bones = first_face.has_bones;
        has_weights = first_face.has_weights;
        has_uvs = first_face.has_uvs;
        has_uv2s = first_face.has_uv2s;

        faces_reader = faces.read();

        arrays.resize(Mesh::ARRAY_MAX);

        int array_length = faces.size() * 3;
        vertexes.resize(array_length);
        vertexes_writer = vertexes.write();

        // There's gotta be a less tedious way of doing this
        if (has_normals) {
            normals.resize(array_length);
            normals_writer = normals.write();
        }

        if (has_tangents) {
            tangents.resize(array_length * 4);
            tangents_writer = tangents.write();
        }

        if (has_colors) {
            colors.resize(array_length);
            colors_writer = colors.write();
        }

        if (has_bones) {
            bones.resize(array_length * 4);
            bones_writer = bones.write();
        }

        if (has_weights) {
            weights.resize(array_length * 4);
            weights_writer = weights.write();
        }

        if (has_uvs) {
            uvs.resize(array_length);
            uvs_writer = uvs.write();
        }

        if (has_uv2s) {
            uv2s.resize(array_length);
            uv2s_writer = uv2s.write();
        }
    }

    /**
     * Takes data from the faces using the lookup_idx and stores it
     * to be saved into vertex arrays (see add_to_mesh for how to attach
     * that information into a mesh)
    */
    _FORCE_INLINE_ void fill(int lookup_idx, int set_idx) {
        // TODO - I think the function definition here with lookup_idx and set_idx
        // is reversed from FaceFiller#fill. We should make that more consistant
        //
        // As mentioned in the FaceFiller comments, while having this function work
        // on a vertex by vertex basis helps with cleaner code (especially, in this case,
        // when it comes to reversing the order of cross section verts), its conceptually
        // and perhaps performancely drawnback back by having to do these repeated calculations
        // and boolean checks (I'd hope the force_inline would help with the function invocation
        // cost but even then who knows).
        int face_idx = lookup_idx / 3;
        int idx_offset = lookup_idx % 3;

        SlicerFace face = faces_reader[face_idx];

        vertexes_writer[set_idx] = face.vertex[idx_offset];

        if (has_normals) {
            normals_writer[set_idx] = face.normal[idx_offset];
        }

        if (has_tangents) {
            tangents_writer[set_idx * 4] = face.tangent[idx_offset][0];
            tangents_writer[set_idx * 4 + 1] = face.tangent[idx_offset][1];
            tangents_writer[set_idx * 4 + 2] = face.tangent[idx_offset][2];
            tangents_writer[set_idx * 4 + 3] = face.tangent[idx_offset][3];
        }

        if (has_colors) {
            colors_writer[set_idx] = face.color[idx_offset];
        }

        if (has_bones) {
            bones_writer[set_idx * 4] = face.bones[idx_offset][0];
            bones_writer[set_idx * 4 + 1] = face.bones[idx_offset][1];
            bones_writer[set_idx * 4 + 2] = face.bones[idx_offset][2];
            bones_writer[set_idx * 4 + 3] = face.bones[idx_offset][3];
        }

        if (has_weights) {
            weights_writer[set_idx * 4] = face.weights[idx_offset][0];
            weights_writer[set_idx * 4 + 1] = face.weights[idx_offset][1];
            weights_writer[set_idx * 4 + 2] = face.weights[idx_offset][2];
            weights_writer[set_idx * 4 + 3] = face.weights[idx_offset][3];
        }

        if (has_uvs) {
            uvs_writer[set_idx] = face.uv[idx_offset];
        }

        if (has_uv2s) {
            uv2s_writer[set_idx] = face.uv2[idx_offset];
        }
    }

    /**
     * Adds the vertex information read from the "fill" as a new surface
     * of the passed in mesh and sets the passed in material to the new
     * surface
    */
    void add_to_mesh(ArrayMesh &mesh, Ref<Material> material) {
        arrays[Mesh::ARRAY_VERTEX] = vertexes;

        if (has_normals)
            arrays[Mesh::ARRAY_NORMAL] = normals;
        
        if (has_tangents)
            arrays[Mesh::ARRAY_TANGENT] = tangents;

        if (has_colors)
            arrays[Mesh::ARRAY_COLOR] = colors;

        if (has_bones)
            arrays[Mesh::ARRAY_BONES] = bones;
        
        if (has_weights)
            arrays[Mesh::ARRAY_WEIGHTS] = weights;
        
        if (has_uvs)
            arrays[Mesh::ARRAY_TEX_UV] = uvs;

        if (has_uv2s)
            arrays[Mesh::ARRAY_TEX_UV2] = uv2s;

        mesh.add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
        mesh.surface_set_material(mesh.get_surface_count() - 1, material);
    }

    ~SurfaceFiller() {
        vertexes_writer.release();

        if (has_normals)
            normals_writer.release();

        if (has_tangents)
            tangents_writer.release();
        
        if (has_colors)
            colors_writer.release();
        
        if (has_bones)
            bones_writer.release();
        
        if (has_weights)
            weights_writer.release();

        if (has_uvs)
            uvs_writer.release();
        
        if (has_uv2s)
            uv2s_writer.release();
    }
};

#endif // SURFACE_FILLER_H
