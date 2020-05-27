#include "../catch.hpp"
#include "../../utils/slicer_face.h"
#include "scene/resources/primitive_meshes.h"

float rand(int max) {
    return Math::random(0, max);
}

int rounded() {
    return Math::round(rand(1));
}

Array make_test_array(int faces) {
    Array arrays;
    arrays.resize(Mesh::ARRAY_MAX);

    PoolVector<Vector3> points;
    PoolVector<Vector3> normals;
    PoolVector<Color> colors;
    PoolVector<real_t> tangents;
    PoolVector<Vector2> uvs;

    for (int i = 0; i < faces * 3; i++) {
        points.push_back(Vector3(rand(10), rand(10), rand(10)));
        normals.push_back(Vector3(rounded(), rounded(), rounded()));
        colors.push_back(Color(rounded(), rounded(), rounded(), rounded()));

        tangents.push_back(rounded());
        tangents.push_back(rounded());
        tangents.push_back(rounded());
        tangents.push_back(rounded());

        uvs.push_back(Vector2(rounded(), rounded()));
    }

    arrays[Mesh::ARRAY_VERTEX] = points;
    arrays[Mesh::ARRAY_NORMAL] = normals;
    arrays[Mesh::ARRAY_COLOR] = colors;
    arrays[Mesh::ARRAY_TANGENT] = tangents;
    arrays[Mesh::ARRAY_TEX_UV] = uvs;

    return arrays;
}

TEST_CASE( "[SlicerFace]" ) {
    SECTION( "Parses faces similar to built in method" ) {
        SphereMesh sphere_mesh;
        auto control_faces = sphere_mesh.get_faces();
        PoolVector<SlicerFace> faces = SlicerFace::faces_from_surface(sphere_mesh, 0);
        REQUIRE( faces.size() == control_faces.size() );
        for (int i = 0; i < faces.size(); i++) {
            REQUIRE( faces[i] == control_faces[i] );
        }
    }

    SECTION( "With non indexed arrays" ) {
        ArrayMesh array_mesh;
        Array arrays = make_test_array(3);
        array_mesh.add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
        PoolVector<SlicerFace> faces = SlicerFace::faces_from_surface(array_mesh, 0);
        REQUIRE( faces.size() == 3 );

        PoolVector<Vector3> points = arrays[Mesh::ARRAY_VERTEX];
        PoolVector<Vector3> normals = arrays[Mesh::ARRAY_NORMAL];
        PoolVector<Color> colors = arrays[Mesh::ARRAY_COLOR];
        PoolVector<real_t> tangents = arrays[Mesh::ARRAY_TANGENT];
        PoolVector<Vector2> uvs = arrays[Mesh::ARRAY_TEX_UV];

        for (int i = 0; i < 3; i++) {
            REQUIRE( faces[i].has_normals == true );
            REQUIRE( faces[i].has_tangents == true );
            REQUIRE( faces[i].has_colors == true );
            REQUIRE( faces[i].has_bones == false );
            REQUIRE( faces[i].has_weights == false );
            REQUIRE( faces[i].has_uvs == true );
            REQUIRE( faces[i].has_uv2s == false );

            for (int j = 0; j < 3; j++) {
                REQUIRE( faces[i].vertex[j] == points[i * 3 + j].snapped(Vector3(0.0001, 0.0001, 0.0001)) );
                REQUIRE( faces[i].normal[j] == normals[i * 3 + j]);
                REQUIRE( faces[i].color[j] == colors[i * 3 + j]);

                REQUIRE( faces[i].tangent[j][0] == tangents[(i * 3 * 4) + (j * 4)] );
                REQUIRE( faces[i].tangent[j][1] == tangents[(i * 3 * 4) + (j * 4) + 1] );
                REQUIRE( faces[i].tangent[j][2] == tangents[(i * 3 * 4) + (j * 4) + 2] );
                REQUIRE( faces[i].tangent[j][3] == tangents[(i * 3 * 4) + (j * 4) + 3] );

                REQUIRE( faces[i].uv[j] == uvs[i * 3 + j]);
            }
        }
    }

    SECTION( "With indexed arrays" ) {
        ArrayMesh array_mesh;
        Array arrays = make_test_array(24);
        int idxs[9] = {1, 4, 7, 10, 13, 14, 17, 19, 21};
        PoolVector<int> indices;
        for (int i = 0; i < 9; i++) {
            indices.push_back(idxs[i]);
        }

        arrays[Mesh::ARRAY_INDEX] = indices;
        array_mesh.add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
        PoolVector<SlicerFace> faces = SlicerFace::faces_from_surface(array_mesh, 0);
        REQUIRE( faces.size() == 3 );

        PoolVector<Vector3> points = arrays[Mesh::ARRAY_VERTEX];
        PoolVector<Vector3> normals = arrays[Mesh::ARRAY_NORMAL];
        PoolVector<Color> colors = arrays[Mesh::ARRAY_COLOR];
        PoolVector<real_t> tangents = arrays[Mesh::ARRAY_TANGENT];
        PoolVector<Vector2> uvs = arrays[Mesh::ARRAY_TEX_UV];

        for (int i = 0; i < 9; i++) {
            int face = i / 3;
            REQUIRE( faces[face].has_normals == true );
            REQUIRE( faces[face].has_tangents == true );
            REQUIRE( faces[face].has_colors == true );
            REQUIRE( faces[face].has_bones == false );
            REQUIRE( faces[face].has_weights == false );
            REQUIRE( faces[face].has_uvs == true );
            REQUIRE( faces[face].has_uv2s == false );

            REQUIRE( faces[face].vertex[i % 3] == points[idxs[i]].snapped(Vector3(0.0001, 0.0001, 0.0001)) );
            REQUIRE( faces[face].normal[i % 3] == normals[idxs[i]] );
            REQUIRE( faces[face].color[i % 3] == colors[idxs[i]] );

            REQUIRE( faces[face].tangent[i % 3][0] == tangents[(idxs[i] * 4)] );
            REQUIRE( faces[face].tangent[i % 3][1] == tangents[(idxs[i] * 4) + 1] );
            REQUIRE( faces[face].tangent[i % 3][2] == tangents[(idxs[i] * 4) + 2] );
            REQUIRE( faces[face].tangent[i % 3][3] == tangents[(idxs[i] * 4) + 3] );

            REQUIRE( faces[face].uv[i % 3] == uvs[idxs[i]] );
        }
    }
}
