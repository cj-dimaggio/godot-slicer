#ifndef SLICER_VECTOR4_H
#define SLICER_VECTOR4_H

#include "core/math/math_funcs.h"
#include "core/ustring.h"

/**
 * Godot does not currently have a 4 dimensional Vector class so we just
 * throw one together ourselves. We're not looking for much more than a
 * simple container to make the code for vertex tangents, bones, and weights
 * cleaner. Functionality can be added piecemeal as needed.
 * 
 * TODO - Would it be better just to use the Godot Color class instead? Functionally
 * I believe the only difference would be that Color doesn't use `real_t` so values
 * might have lower precision. It also might just be a bit misleading.
*/
struct SlicerVector4 {

	enum Axis {
		AXIS_X,
		AXIS_Y,
		AXIS_Z,
		AXIS_W,
	};

	union {
		struct {
			real_t x;
			real_t y;
			real_t z;
			real_t w;
		};

		real_t coord[4];
	};

	_FORCE_INLINE_ const real_t &operator[](int p_axis) const {

		return coord[p_axis];
	}

	_FORCE_INLINE_ real_t &operator[](int p_axis) {

		return coord[p_axis];
	}


	SlicerVector4 operator*(real_t scalar) const {
		return SlicerVector4(x * scalar, y * scalar, z * scalar, w * scalar);
	}

	SlicerVector4 operator+(const SlicerVector4 &other) const {
		return SlicerVector4(x + other.x, y + other.y, z + other.z, w + other.w);
	}

	operator String() const {
        return (rtos(x) + ", " + rtos(y) + ", " + rtos(z), + ", " + rtos(w));
    }

	_FORCE_INLINE_ SlicerVector4(real_t p_x, real_t p_y, real_t p_z, real_t p_w) {
		x = p_x;
		y = p_y;
		z = p_z;
        w = p_w;
	}
	_FORCE_INLINE_ SlicerVector4() { x = y = z = w = 0; }
};

#endif // SLICER_VECTOR4_H
