#pragma once
#include <string>
#include <vector>
#include <unordered_set>

#include "geo.h"

namespace transport_catalogue{

    struct Stop {
        std::string name_;
        geo::Coordinates coordinates_;

        Stop() = default;
        Stop(const std::string& name, const geo::Coordinates& coordinates);
        bool operator==(const Stop& stop) const;
        operator uintptr_t() const {
            uintptr_t a = reinterpret_cast<uintptr_t>(this);
            return a;
        }
    };

    struct Bus {
        std::string name_;
        std::vector<const Stop*> route_;
        std::unordered_set <uintptr_t, std::hash<uintptr_t>> uniqueStops;
        bool isRoundtrip_;

        Bus() = default;
        Bus(const std::string& name, const std::vector<const Stop*>& route, bool isRoundtrip);
        bool operator==(const Bus& bus) const;
    };

    struct BusInfo {
        const std::string_view name_;
        const size_t stopsAmount_;
        const size_t uniqueStopsAmount_;
        const double routeLength_;
        const double curvature_;

        BusInfo(const std::string_view name, const size_t stopsAmount,
                const size_t uniqueStopsAmount, const double routeLength, const double curvature);
        bool operator==(const BusInfo& busInfo) const;
    };

    struct StopQuery {
        StopQuery(const std::string& name, double latitude, double longitude,
                  const std::vector<std::pair<std::string, int>>& distance_to_stops);
        std::string name_;
        double latitude_;
        double longitude_;
        std::vector<std::pair<std::string, int>> distance_to_stops_;
    };

    struct BusQuery {
        BusQuery(const std::string &name, const std::vector<std::string>& stopNames, bool isRingRoute);
        std::string name_;
        std::vector<std::string> stopNames_;
        bool isRoundtrip_;
    };
}
