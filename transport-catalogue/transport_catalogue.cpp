#include "transport_catalogue.h"


using transport_catalogue::TransportCatalogue;
using transport_catalogue::Bus;
using transport_catalogue::Stop;
using transport_catalogue::BusInfo;


void TransportCatalogue::AddStop(const Stop& stop) {
    Stop& ref = stops_.emplace_back(stop);
    stopByName_.insert({ref.name_, ref});
    busesByStopName[ref.name_];
}
void TransportCatalogue::AddBus(const Bus& bus) {
    Bus& ref = buses_.emplace_back(bus);
    busByName_.insert({ref.name_, ref});
    for(const Stop* stop : bus.route_) {
        busesByStopName[stop->name_].insert(ref.name_);
    }
}

void TransportCatalogue::SetStopsDistance(const Stop* stopFrom, const Stop* stopTo, int distance) {
    stopDistances_[{stopFrom, stopTo}] = distance;
}

const Bus& TransportCatalogue::GetBus(const std::string& busName) const {
    return busByName_.at(busName);
}
const Stop& TransportCatalogue::GetStop(const std::string& stopName) const {
    return stopByName_.at(stopName);
}
BusInfo TransportCatalogue::GetBusInfo(const std::string& busName) const{
    const Bus& bus = GetBus(busName);
    double geo_distance = ComputeRouteDistance(bus);
    double real_distance = ComputeRealRouteDistance(bus);
    return {bus.name_, bus.route_.size(),
            bus.uniqueStops.size(), real_distance, real_distance/geo_distance};
}

const std::unordered_map<std::string_view, const Bus&, std::hash<std::string_view>>&
TransportCatalogue::GetAllBuses() const {
    return busByName_;
}

const std::deque<Stop>& TransportCatalogue::GetAllStops() const {
    return stops_;
}

const std::set<std::string_view> TransportCatalogue::GetStopInfo(const std::string& name) const {
    return busesByStopName.at(name);
}

double TransportCatalogue::ComputeRouteDistance(const Bus& bus) const {
    double distance = 0;
    for(size_t i = 0; i + 1 < bus.route_.size(); ++i) {
        distance += ComputeDistance(bus.route_[i]->coordinates_, bus.route_[i + 1]->coordinates_);
    }
    return distance;
}
double TransportCatalogue::ComputeRealRouteDistance(const Bus& bus) const {
    double distance = 0;
    for(size_t i = 0; i + 1 < bus.route_.size(); ++i) {
        if(stopDistances_.count({bus.route_[i], bus.route_[i]})) {
            distance += stopDistances_.at({bus.route_[i], bus.route_[i]});
        }
        if(stopDistances_.count({bus.route_[i], bus.route_[i + 1]})) {
            distance += stopDistances_.at({bus.route_[i], bus.route_[i + 1]});
        } else if(stopDistances_.count({bus.route_[i + 1], bus.route_[i]})) {
            distance += stopDistances_.at({bus.route_[i + 1], bus.route_[i]});
        } else {
            distance += ComputeDistance(bus.route_[i]->coordinates_, bus.route_[i + 1]->coordinates_);
        }
    }
    return distance;
}

void transport_catalogue::tests::AddingStop(TransportCatalogue& catalogue) {
    using namespace std::literals;
    catalogue.AddStop({"Stop1"s, {53.33333, 54.444444}});
    catalogue.AddStop({"Stop2"s, {54.33333, 54.444444}});
    catalogue.AddStop({"Stop3"s, {55.33333, 54.444444}});
    catalogue.AddStop({"Stop4"s, {56.33333, 54.444444}});
    catalogue.AddStop({"Stop5"s, {57.33333, 54.444444}});
    catalogue.AddStop({"Stop6"s, {58.33333, 54.444444}});

    assert(catalogue.GetStop("Stop1"s) == Stop("Stop1"s, {53.33333, 54.444444}));
    assert(catalogue.GetStop("Stop6"s) == Stop("Stop6"s, {58.33333, 54.444444}));
    assert(catalogue.GetStop("Stop4"s) == Stop("Stop4"s, {56.33333, 54.444444}));
    assert(catalogue.GetStop("Stop5"s) == Stop("Stop5"s, {57.33333, 54.444444}));
    assert(catalogue.GetStop("Stop2"s) == Stop("Stop2"s, {54.33333, 54.444444}));
}
void transport_catalogue::tests::AddingBus(TransportCatalogue& catalogue) {
    AddingStop(catalogue);
    using namespace std::literals;

    catalogue.AddBus({"route66"s, {&catalogue.GetStop("Stop1"s),
                                   &catalogue.GetStop("Stop4"s),
                                   &catalogue.GetStop("Stop5"s),
                                   &catalogue.GetStop("Stop2"s),
                                   &catalogue.GetStop("Stop5"s)}, false});

    catalogue.AddBus({"route2"s, {&catalogue.GetStop("Stop4"s),
                                  &catalogue.GetStop("Stop3"s),
                                  &catalogue.GetStop("Stop2"s),
                                  &catalogue.GetStop("Stop1"s)}, false});

    catalogue.AddBus({"route8"s, {&catalogue.GetStop("Stop5"s),
                                  &catalogue.GetStop("Stop4"s),
                                  &catalogue.GetStop("Stop1"s),
                                  &catalogue.GetStop("Stop2"s)}, false});

    assert(catalogue.GetBus("route66"s) == Bus("route66"s, {&catalogue.GetStop("Stop1"s),
                                                            &catalogue.GetStop("Stop4"s),
                                                            &catalogue.GetStop("Stop5"s),
                                                            &catalogue.GetStop("Stop2"s),
                                                            &catalogue.GetStop("Stop5"s)}, false));

    assert(catalogue.GetBus("route8"s) == Bus("route8"s, {&catalogue.GetStop("Stop5"s),
                                                          &catalogue.GetStop("Stop4"s),
                                                          &catalogue.GetStop("Stop1"s),
                                                          &catalogue.GetStop("Stop2"s)}, false));

    assert(catalogue.GetBus("route2"s) == Bus("route2"s, {&catalogue.GetStop("Stop4"s),
                                                          &catalogue.GetStop("Stop3"s),
                                                          &catalogue.GetStop("Stop2"s),
                                                          &catalogue.GetStop("Stop1"s)}, false));
}
void transport_catalogue::tests::GettingBusInfo(TransportCatalogue& catalogue) {
    AddingBus(catalogue);
    using namespace std::literals;
    BusInfo busInfo = catalogue.GetBusInfo("route66"s);

    Bus bus = catalogue.GetBus("route66"s);

    double geo_length = ComputeDistance(bus.route_[0]->coordinates_, bus.route_[1]->coordinates_) +
                        ComputeDistance(bus.route_[1]->coordinates_, bus.route_[2]->coordinates_) +
                        ComputeDistance(bus.route_[2]->coordinates_, bus.route_[3]->coordinates_) +
                        ComputeDistance(bus.route_[3]->coordinates_, bus.route_[4]->coordinates_);

    assert(busInfo == BusInfo("route66"s, 5, 4, geo_length, catalogue.ComputeRealRouteDistance(bus)/geo_length));
}