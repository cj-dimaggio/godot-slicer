#include "catch.hpp"
#include "../slicer.h"
#include "scene/resources/primitive_meshes.h"

TEST_CASE( "[Slicer]" ) {
    Plane plane(Vector3(1, 0, 0), 0);

    SECTION( "Smoke test" ) {
        Ref<SphereMesh> sphere_mesh;
        sphere_mesh.instance();
        Slicer slicer;
        Ref<SlicedMesh> sliced_mesh= slicer.slice_by_plane(sphere_mesh, plane, NULL);
        REQUIRE( sliced_mesh.is_null() == false );
    }
}
