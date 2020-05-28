#ifndef TRIANGULATOR_H
#define TRIANGULATOR_H

#include "slicer_face.h"

/**
 * Contains functions related to performing generative
 * operations on points
*/
namespace Triangulator {
    /**
     * Calculates the area of a 2 dimensional triangle
    */
    real_t tri_area_2d(real_t x1, real_t y1, real_t x2, real_t y2, real_t x3, real_t y3);

    /**
     * Uses a monotone chain algorithm to generate the faces of a convex hull from a set of points
    */
    PoolVector<SlicerFace> monotone_chain(const PoolVector<Vector3> &interception_points, Vector3 plane_normal);
} // Triangulator


#endif // TRIANGULATOR_H
