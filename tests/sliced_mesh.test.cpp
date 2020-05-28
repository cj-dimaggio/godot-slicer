#include "catch.hpp"
#include "../sliced_mesh.h"

TEST_CASE( "[SlicedMesh]") {
    SECTION("Creates new meshes") {
        Intersector::SplitResult result;
        PoolVector<Intersector::SplitResult> results;
        result.material = Ref<SpatialMaterial>();
        result.lower_faces.push_back(SlicerFace(Vector3(0, 0, 0), Vector3(0, 1, 0), Vector3(0, 1, 1)));
        result.lower_faces.push_back(SlicerFace(Vector3(0, 1, 1), Vector3(0, 0, 1), Vector3(0, 0, 0)));

        result.upper_faces.push_back(SlicerFace(Vector3(0, 1, 0), Vector3(0, 2, 0), Vector3(0, 2, 1)));
        result.upper_faces.push_back(SlicerFace(Vector3(0, 2, 1), Vector3(0, 1, 1), Vector3(0, 1, 0)));

        results.push_back(result);

        PoolVector<SlicerFace> cross_section_faces;
        Ref<SpatialMaterial> cross_section_material;
        cross_section_faces.push_back(SlicerFace(Vector3(0, 1, 0), Vector3(1, 1, 0), Vector3(0, 1, 1)));

        SlicedMesh sliced(results, cross_section_faces, cross_section_material);
        REQUIRE_FALSE(sliced.lower_mesh.is_null());
        REQUIRE_FALSE(sliced.upper_mesh.is_null());

        REQUIRE(sliced.lower_mesh->get_surface_count() == 2);
        REQUIRE(sliced.upper_mesh->get_surface_count() == 2);

        REQUIRE(sliced.lower_mesh->surface_get_material(0) == result.material);
        REQUIRE(sliced.lower_mesh->surface_get_material(1) == cross_section_material);

        REQUIRE(sliced.upper_mesh->surface_get_material(0) == result.material);
        REQUIRE(sliced.upper_mesh->surface_get_material(1) == cross_section_material);
    }
}
