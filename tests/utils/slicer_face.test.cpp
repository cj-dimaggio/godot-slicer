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
    SECTION("faces_from_surface") {
        SECTION( "Parses faces similar to built in method") {
            SphereMesh sphere_mesh;
            auto control_faces = sphere_mesh.get_faces();
            PoolVector<SlicerFace> faces = SlicerFace::faces_from_surface(sphere_mesh, 0);
            REQUIRE( faces.size() == control_faces.size() );
            for (int i = 0; i < faces.size(); i++) {
                REQUIRE( faces[i] == control_faces[i] );
            }
        }

        SECTION( "With non indexed arrays") {
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

        SECTION( "With indexed arrays") {
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
    SECTION("sub_face") {
    }

    SECTION("set_uvs") {
        SlicerFace face;
        face.set_uvs(Vector2(0, 0), Vector2(0.5, 0.5), Vector2(1, 1));
        REQUIRE(face.has_uvs);
        REQUIRE(face.uv[0] == Vector2(0, 0));
        REQUIRE(face.uv[1] == Vector2(0.5, 0.5));
        REQUIRE(face.uv[2] == Vector2(1, 1));
    }

    SECTION("set_normals") {
        SlicerFace face;
        face.set_normals(Vector3(0, 0, 0), Vector3(0.5, 0.5, 0.5), Vector3(1, 1, 1));
        REQUIRE(face.has_normals);
        REQUIRE(face.normal[0] == Vector3(0, 0, 0));
        REQUIRE(face.normal[1] == Vector3(0.5, 0.5, 0.5));
        REQUIRE(face.normal[2] == Vector3(1, 1, 1));
    }

    SECTION("set_tangents") {
        SlicerFace face;
        face.set_tangents(SlicerVector4(0, 0, 0, 0), SlicerVector4(0.5, 0.5, 0.5, 0.5), SlicerVector4(1, 1, 1, 1));
        REQUIRE(face.has_tangents);
        REQUIRE(face.tangent[0] == SlicerVector4(0, 0, 0, 0));
        REQUIRE(face.tangent[1] == SlicerVector4(0.5, 0.5, 0.5, 0.5));
        REQUIRE(face.tangent[2] == SlicerVector4(1, 1, 1, 1));
    }

    SECTION("set_colors") {
        SlicerFace face;
        face.set_colors(Color(0, 0, 0, 0), Color(0.5, 0.5, 0.5, 0.5), Color(1, 1, 1, 1));
        REQUIRE(face.has_colors);
        REQUIRE(face.color[0] == Color(0, 0, 0, 0));
        REQUIRE(face.color[1] == Color(0.5, 0.5, 0.5, 0.5));
        REQUIRE(face.color[2] == Color(1, 1, 1, 1));
    }

    SECTION("set_bones") {
        SlicerFace face;
        face.set_bones(SlicerVector4(0, 0, 0, 0), SlicerVector4(0.5, 0.5, 0.5, 0.5), SlicerVector4(1, 1, 1, 1));
        REQUIRE(face.has_bones);
        REQUIRE(face.bones[0] == SlicerVector4(0, 0, 0, 0));
        REQUIRE(face.bones[1] == SlicerVector4(0.5, 0.5, 0.5, 0.5));
        REQUIRE(face.bones[2] == SlicerVector4(1, 1, 1, 1));
    }

    SECTION("set_weights") {
        SlicerFace face;
        face.set_weights(SlicerVector4(0, 0, 0, 0), SlicerVector4(0.5, 0.5, 0.5, 0.5), SlicerVector4(1, 1, 1, 1));
        REQUIRE(face.has_weights);
        REQUIRE(face.weights[0] == SlicerVector4(0, 0, 0, 0));
        REQUIRE(face.weights[1] == SlicerVector4(0.5, 0.5, 0.5, 0.5));
        REQUIRE(face.weights[2] == SlicerVector4(1, 1, 1, 1));
    }

    SECTION("set_uv2s") {
        SlicerFace face;
        face.set_uv2s(Vector2(0, 0), Vector2(0.5, 0.5), Vector2(1, 1));
        REQUIRE(face.has_uv2s);
        REQUIRE(face.uv2[0] == Vector2(0, 0));
        REQUIRE(face.uv2[1] == Vector2(0.5, 0.5));
        REQUIRE(face.uv2[2] == Vector2(1, 1));
    }
}
