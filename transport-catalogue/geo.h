#pragma once
#define _USE_MATH_DEFINES
#include <cmath>

namespace geo {

    struct Coordinates {
        double lat; // Широта
        double lng; // Долгота
        bool operator==(const Coordinates& other) const {
            return lat == other.lat && lng == other.lng;
        }
        bool operator!=(const Coordinates& other) const {
            return !(*this == other);
        }
    };

    double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace geo