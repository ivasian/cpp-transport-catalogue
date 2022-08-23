#pragma once

#include <cmath>
#include <limits>

struct Coordinates {
    double lat = 0.0;
    double lng = 0.0;

    bool IsDoubleEqual(double x, double y) const {
        return std::fabs(x - y) < std::numeric_limits<double>::epsilon();
    }

    bool operator==(const Coordinates& other) const {
        return IsDoubleEqual(lat, other.lat) && IsDoubleEqual(lng, other.lng);
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};

inline double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    if (from == to) {
        return 0;
    }
    static const double dr = 3.1415926535 / 180.;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
           * 6371000;
}