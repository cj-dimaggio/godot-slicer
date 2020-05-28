#include "../catch.hpp"
#include "../../utils/triangulator.h"

TEST_CASE( "[triangulator]" ) {
    SECTION("monotone_chain") {
        PoolVector<Vector3> interception_points;
        interception_points.push_back(Vector3(0, 0, 0));
        interception_points.push_back(Vector3(1, 0, 0));
        interception_points.push_back(Vector3(1, 0, 1));
        interception_points.push_back(Vector3(0, 0, 1));
        interception_points.push_back(Vector3(0.5, 0, 0.5));

        PoolVector<SlicerFace> faces = Triangulator::monotone_chain(interception_points, Vector3(0, 1, 0));
        REQUIRE(faces.size() == 2);
        REQUIRE(faces[0] == SlicerFace(Vector3(1, 0, 1), Vector3(0, 0, 1), Vector3(0, 0, 0)));
        REQUIRE(faces[1] == SlicerFace(Vector3(1, 0, 1), Vector3(0, 0, 0), Vector3(1, 0, 0)));

        REQUIRE((faces[0].has_normals && faces[0].has_uvs && faces[0].has_tangents));
        REQUIRE((faces[1].has_normals && faces[1].has_uvs && faces[1].has_tangents));

        REQUIRE((faces[0].normal[0] == Vector3(0, 1, 0) && faces[0].normal[1] == Vector3(0, 1, 0) && faces[0].normal[2] == Vector3(0, 1, 0)));
        REQUIRE((faces[1].normal[0] == Vector3(0, 1, 0) && faces[1].normal[1] == Vector3(0, 1, 0) && faces[1].normal[2] == Vector3(0, 1, 0)));

        REQUIRE((faces[0].uv[0] == Vector2(0, 0) && faces[0].uv[1] == Vector2(1, 0) && faces[0].uv[2] == Vector2(1, 1)));
        REQUIRE((faces[1].uv[0] == Vector2(0, 0) && faces[1].uv[1] == Vector2(1, 1) && faces[1].uv[2] == Vector2(0, 1)));

        REQUIRE(faces[0].tangent[0] == SlicerVector4(-1, 0, 0, -1));
        REQUIRE(faces[0].tangent[1] == SlicerVector4(-1, 0, 0, -1));
        REQUIRE(faces[0].tangent[2] == SlicerVector4(-1, 0, 0, -1));

        REQUIRE(faces[1].tangent[0] == SlicerVector4(-1, 0, 0, -1));
        REQUIRE(faces[1].tangent[1] == SlicerVector4(-1, 0, 0, -1));
        REQUIRE(faces[1].tangent[2] == SlicerVector4(-1, 0, 0, -1));
    }
}