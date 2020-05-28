#include "../catch.hpp"
#include "../../utils/slicer_vector4.h"

TEST_CASE( "[SlicerVector4]" ) {
    SECTION( "x,y,z,w maps to coord" ) {
        SlicerVector4 vec = SlicerVector4(0.5, 1.5, 2.5, 3.5);
        REQUIRE( vec.x == 0.5 );
        REQUIRE( vec.y == 1.5 );
        REQUIRE( vec.z == 2.5 );
        REQUIRE( vec.w == 3.5 );

        REQUIRE( vec.coord[0] == 0.5 );
        REQUIRE( vec.coord[1] == 1.5 );
        REQUIRE( vec.coord[2] == 2.5 );
        REQUIRE( vec.coord[3] == 3.5 );

        vec.x = 5.0;

        REQUIRE( vec.x == 5.0 );
        REQUIRE( vec.coord[0] == 5.0 );
    }

    SECTION( "operator*" ) {
        SlicerVector4 vec = SlicerVector4(1, 2, 3, 4) * 2;

        REQUIRE(vec.x == 2);
        REQUIRE(vec.y == 4);
        REQUIRE(vec.z == 6);
        REQUIRE(vec.w == 8);
    }

    SECTION( "operator*+" ) {
        SlicerVector4 vec = SlicerVector4(1, 2, 3, 4) + SlicerVector4(2, 3, 4, 5);

        REQUIRE(vec.x == 3);
        REQUIRE(vec.y == 5);
        REQUIRE(vec.z == 7);
        REQUIRE(vec.w == 9);
    }

    SECTION( "operator==" ) {
        REQUIRE(SlicerVector4(1, 1, 1, 1) == SlicerVector4(1, 1, 1, 1));
        REQUIRE_FALSE(SlicerVector4(1, 1, 1, 1) == SlicerVector4(2, 1, 1, 1));
        REQUIRE_FALSE(SlicerVector4(1, 1, 1, 1) == SlicerVector4(1, 2, 1, 1));
        REQUIRE_FALSE(SlicerVector4(1, 1, 1, 1) == SlicerVector4(1, 1, 2, 1));
        REQUIRE_FALSE(SlicerVector4(1, 1, 1, 1) == SlicerVector4(1, 1, 1, 2));
    }
}
