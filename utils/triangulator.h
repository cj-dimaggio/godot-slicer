#ifndef TRIANGULATOR_H
#define TRIANGULATOR_H

#include "slicer_face.h"

namespace Triangulator {
    real_t tri_area_2d(real_t x1, real_t y1, real_t x2, real_t y2, real_t x3, real_t y3);
    PoolVector<SlicerFace> monotone_chain(const PoolVector<Vector3> &interception_points, Vector3 plane_normal);
} // Triangulator


#endif // TRIANGULATOR_H
