#pragma once
#include "transport_catalogue.h"


namespace transport_router {

    using transport_catalogue::Stop;
    using transport_catalogue::Bus;
    using transport_catalogue::TransportCatalogue;
    using Graph = graph::DirectedWeightedGraph<double>;
    using Router = graph::Router<double>;

    struct RoutingSetting {
        double busWaitTime;
        double busVelocity;
    };

    struct PassengerActivity {
        bool isWait;
        double time;

        const Stop* stop;
        const Bus* bus;
        int spanCount;
    };

    struct RouteInfo {
        double totalTime;
        std::vector<PassengerActivity> routeSteps;
    };

    class TransportRouter {


    public:
        TransportRouter(const TransportCatalogue& db, RoutingSetting routingSetting);

        Graph& BuildGraph();

        std::optional<RouteInfo> GetOptimalRoute(std::string& routeFrom, std::string& routeTo) const;

    private:
        std::map<int, PassengerActivity> edgeIds_;
        std::map<const Stop*, size_t> vertexIds_;
        Graph graph_;
        const TransportCatalogue& db_;
        const RoutingSetting routingSetting_;
        const Router router_;
    };

}
