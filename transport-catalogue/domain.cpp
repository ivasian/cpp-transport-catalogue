#include "domain.h"

namespace transport_catalogue {
    Stop::Stop(const std::string& name, const geo::Coordinates& coordinates) :
            name_(name), coordinates_(coordinates){
    }

    bool Stop::operator==(const Stop& stop) const {
        return name_ == stop.name_ && coordinates_ == stop.coordinates_;
    }

    Bus::Bus(const std::string& name, const std::vector<const Stop*>& route, bool isRoundtrip) :
            name_(name), route_(route), isRoundtrip_(isRoundtrip) {
        for(auto stop : route_) {
            uniqueStops.insert(reinterpret_cast<uintptr_t>(stop));
        }
    }

    bool Bus::operator==(const Bus& bus) const {
        return name_ == bus.name_ && route_ == bus.route_;
    }

    BusInfo::BusInfo(const std::string_view name, const size_t stopsAmount,
                     const size_t uniqueStopsAmount, const double routeLength, const double curvature) :
            name_(name), stopsAmount_(stopsAmount),
            uniqueStopsAmount_(uniqueStopsAmount), routeLength_(routeLength), curvature_(curvature){
    }

    bool BusInfo::operator==(const BusInfo& busInfo) const {
        return name_ == busInfo.name_ && stopsAmount_ == busInfo.stopsAmount_ &&
               uniqueStopsAmount_ == busInfo.uniqueStopsAmount_  && routeLength_ == busInfo.routeLength_
               && curvature_ == busInfo.curvature_;
    }

    StopQuery::StopQuery(const std::string& name, double latitude, double longitude, const std::vector<std::pair<std::string, int>>& distance_to_stops)
            : name_(move(name)), latitude_(latitude), longitude_(longitude),
              distance_to_stops_(move(distance_to_stops)){
    }

    BusQuery::BusQuery(const std::string &name, const std::vector<std::string>& stopNames, bool isRoundtrip)
            : name_(name), stopNames_(stopNames), isRoundtrip_(isRoundtrip) {
    }
}
