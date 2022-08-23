#pragma once
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <algorithm>
#include <functional>
#include <cassert>
#include <sstream>

#include "geo.h"

namespace transport_catalogue {

struct Bus;

struct Stop {
    std::string name_;
    Coordinates coordinates_;

    Stop() = default;

    Stop(const std::string& name, const Coordinates& coordinates) :
            name_(name), coordinates_(coordinates){
    };

    bool operator==(const Stop& stop) const {
        return name_ == stop.name_ && coordinates_ == stop.coordinates_;
    }

};
struct Bus {
    std::string name_;
    std::vector<const Stop*> route_;
    std::unordered_set <uintptr_t, std::hash<uintptr_t>> uniqueStops;

    Bus() = default;

    Bus(const std::string& name, const std::vector<const Stop*>& route) :
            name_(name), route_(route) {
        for(auto stop : route_) {
            uniqueStops.insert(reinterpret_cast<uintptr_t>(stop));
        }
    }

    bool operator==(const Bus& bus) const {
        return name_ == bus.name_ && route_ == bus.route_;
    }
};

struct BusInfo {
    const std::string_view name_;
    const size_t stopsAmount_;
    const size_t uniqueStopsAmount_;
    const double routeLength_;
    const double curvature_;

    BusInfo(const std::string_view name, const size_t stopsAmount,
                const size_t uniqueStopsAmount, const double routeLength, const double curvature) :
            name_(name), stopsAmount_(stopsAmount),
                uniqueStopsAmount_(uniqueStopsAmount), routeLength_(routeLength), curvature_(curvature){
    }

    bool operator==(const BusInfo& busInfo) const {
        return name_ == busInfo.name_ && stopsAmount_ == busInfo.stopsAmount_ &&
                uniqueStopsAmount_ == busInfo.uniqueStopsAmount_  && routeLength_ == busInfo.routeLength_
                && curvature_ == busInfo.curvature_;
    }
};


class TransportCatalogue {

public:
    TransportCatalogue() = default;

    void AddStop(const Stop& stop);
    void AddBus(const Bus& bus);
    void SetStopsDistance(const Stop* stopFrom, const Stop* stopTo, int distance);

    const Bus& GetBus(const std::string& busName) const;
    const Stop& GetStop(const std::string& stopName) const;
    BusInfo GetBusInfo(const std::string& busName);
    const std::set<std::string_view> GetStopInfo(const std::string& stopName);

    double ComputeRouteDistance(const Bus& bus) const;
    double ComputeRealRouteDistance(const Bus& bus) const;


private:

    class PairOfPointerHasher {
    public:
        size_t operator()(const std::pair<const Stop*, const Stop*>& pointers) const {
            return reinterpret_cast<uintptr_t>(pointers.first) +
                    reinterpret_cast<uintptr_t>(pointers.second) * 67;
        }
    };

    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, const Stop&, std::hash<std::string_view>> stopByName_;

    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Bus&, std::hash<std::string_view>> busByName_;

    std::unordered_map<std::string_view, std::set<std::string_view>, std::hash<std::string_view>> busesByStopName;

    std::unordered_map<std::pair<const Stop*, const Stop*>, int, PairOfPointerHasher> stopDistances_;

};


namespace tests{
    void AddingStop(TransportCatalogue& catalogue);
    void AddingBus(TransportCatalogue& catalogue);
    void GettingBusInfo(TransportCatalogue& catalogue);
}

}