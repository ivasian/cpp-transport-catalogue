#pragma once
#include "transport_catalogue.h"
#include "json_builder.h"
#include <memory>

namespace transport_router {

    using transport_catalogue::Stop;
    using transport_catalogue::Bus;
    using transport_catalogue::TransportCatalogue;
    using Graph = graph::DirectedWeightedGraph<double>;
    using Router = graph::Router<double>;
    using BusesContaner = std::unordered_map<std::basic_string_view<char>, const transport_catalogue::Bus &, std::hash<std::basic_string_view<char>>>;


    struct RoutingSetting {
        double busWaitTime;
        double busVelocity;
    };

    struct SerializationSetting {
        std::string filename;
    };

    namespace passenger_activity {

        struct Activity {

            Activity(double time);
            virtual void WriteInJsonDict(json::Dict& dict);

        private:
            double time_;
        };

        struct OnWait : Activity {

            OnWait(double time, const Stop* stop);
            void WriteInJsonDict(json::Dict& dict) override ;
        private:
            const Stop* stop_;
        };

        struct OnBus : Activity {

            OnBus(double time, const Bus* bus, int spanCount) ;
            void WriteInJsonDict(json::Dict& dict) override ;
        private:
            const Bus* bus_;
            int spanCount_;
        };
    }

    using passenger_activity::Activity;
    using passenger_activity::OnWait;
    using passenger_activity::OnBus;

    struct RouteInfo {
        double totalTime;
        std::vector<std::shared_ptr<Activity>> routeSteps;
    };

    class TransportRouter {


    public:
        TransportRouter(const TransportCatalogue& db, RoutingSetting routingSetting);
        TransportRouter(const TransportCatalogue& db, RoutingSetting routingSetting, Graph& graph);
        Graph& BuildGraph();

        std::optional<RouteInfo> GetOptimalRoute(const std::string& routeFrom, const std::string& routeTo) const;

        const std::map<const Stop*, size_t>& GetVertexIds() const;
        const std::map<int, std::shared_ptr<Activity>>& GetEdgeIds() const;
        const Graph& GetGraph() const;

    private:
        double ComputeTimeInMinute (double sInMeters, double vInKmh) const;

        void FillGraphWithStops(const std::deque<Stop>& stops, bool isGraphDeserialized = false);
        void FillGraphWithBuses(const BusesContaner& buses, bool isGraphDeserialized = false);

        std::map<int, std::shared_ptr<Activity>> edgeIds_;
        std::map<const Stop*, size_t> vertexIds_;
        Graph graph_;
        const TransportCatalogue& db_;
        const RoutingSetting routingSetting_;
        const Router router_;
    };

}