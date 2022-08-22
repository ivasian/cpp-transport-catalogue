#pragma once
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include "geo.h"
#include <algorithm>
#include <functional>
#include <cassert>
#include <sstream>

namespace transport_catalogue {

struct Bus;

struct Stop{
    std::string name_;
    Coordinates coordinates_;

    Stop() = default;

    Stop(const std::string& name, const Coordinates& coordinates) :
            name_(name), coordinates_(coordinates){
    };

    bool operator == (const Stop& stop) const {
        return name_ == stop.name_ && coordinates_ == stop.coordinates_;
    }

};
struct Bus{
    std::string name_;
    std::vector<const Stop*> route_;

    Bus() = default;

    Bus(const std::string& name, const std::vector<const Stop*>& route) :
            name_(name), route_(route){
    }

    bool operator == (const Bus& bus) const {
        return name_ == bus.name_ && route_ == bus.route_;
    }
};

struct BusInfo{
    const std::string_view name_;
    const size_t stops_amount_;
    const size_t unique_stops_amount_;
    const double route_length_;
    const double curvature_;

    BusInfo(const std::string_view name, const size_t stops_amount,
                const size_t unique_stops_amount, const double route_length, const double curvature) :
            name_(name), stops_amount_(stops_amount),
                unique_stops_amount_(unique_stops_amount), route_length_(route_length), curvature_(curvature){
    }

    bool operator == (const BusInfo& busInfo) const {
        return name_ == busInfo.name_ && stops_amount_ == busInfo.stops_amount_ &&
                unique_stops_amount_ == busInfo.unique_stops_amount_  && route_length_ == busInfo.route_length_
                && curvature_ == busInfo.curvature_;
    }
};


class TransportCatalogue{

public:
    TransportCatalogue() = default;
    void AddStop(const Stop& stop);
    void AddBus(const Bus& bus);
    const Bus& GetBus(const std::string& name) const;
    const Stop& GetStop(const std::string& name) const;
    BusInfo GetBusInfo(const std::string& name);
    const std::set<std::string_view> GetStopInfo(const std::string& name);
    double ComputeRouteDistance(const Bus& bus) const;
    double ComputeRealRouteDistance(const Bus& bus) const;
    void AddStopsDistance(const Stop* stop1, const Stop* stop2, int distance);

private:
    std::deque<Stop> stops_;
    std::map<std::string_view, const Stop&> name_to_stops_;

    std::deque<Bus> buses_;
    std::map<std::string_view, const Bus&> name_to_buses_;

    std::map<std::string_view, std::set<std::string_view>> stop_to_buses_;

    std::map<std::pair<const Stop*, const Stop*>, int> stop_distances_;

};


namespace tests{
    void AddingStop(TransportCatalogue& catalogue);
    void AddingBus(TransportCatalogue& catalogue);
    void GettingBusInfo(TransportCatalogue& catalogue);
}

}