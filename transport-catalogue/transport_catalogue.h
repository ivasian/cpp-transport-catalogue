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
#include <optional>
#include "router.h"
#include "domain.h"
#include <iostream>

namespace transport_catalogue {

    class TransportCatalogue {

    public:
        TransportCatalogue() = default;

        void AddStop(const Stop& stop);
        void AddBus(const Bus& bus);
        void SetStopsDistance(const Stop* stopFrom, const Stop* stopTo, int distance);
        const Bus& GetBus(const std::string& busName) const;
        const Stop& GetStop(const std::string& stopName) const;
        BusInfo GetBusInfo(const std::string& busName) const;
        const std::set<std::string_view> GetStopInfo(const std::string& stopName) const;
        const std::unordered_map<std::string_view, const Bus&, std::hash<std::string_view>>& GetAllBuses() const;
        const std::deque<Stop>& GetAllStops() const;

        double ComputeRouteDistance(const Bus& bus) const;
        double ComputeRealRouteDistance(const Bus& bus) const;
        double ComputeRealStopToStopDistance(const Bus& bus, size_t indexFrom, size_t indexTo) const;

        template<typename InputIt>
        double ComputeRealRouteDistance(InputIt from, InputIt to) const;


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

    namespace tests {
        void AddingStop(TransportCatalogue& catalogue);
        void AddingBus(TransportCatalogue& catalogue);
        void GettingBusInfo(TransportCatalogue& catalogue);
    }
}