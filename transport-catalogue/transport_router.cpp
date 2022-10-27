#include "transport_router.h"

namespace transport_router {
    TransportRouter::TransportRouter(const TransportCatalogue& db, RoutingSetting routingSetting) :
            db_(db), routingSetting_(routingSetting), router_(BuildGraph()){
    }

    Graph& TransportRouter::BuildGraph() {
        auto& stops = db_.GetAllStops();
        graph_ = Graph(stops.size() * 2);
        size_t vertexId = 0;
        for(auto& stop : stops) {
            size_t edgeId = graph_.AddEdge({vertexId, vertexId + 1, routingSetting_.busWaitTime});
            edgeIds_[edgeId] = {true, routingSetting_.busWaitTime, &stop, nullptr,-1};
            vertexIds_[&stop] = vertexId;
            vertexId += 2;
        }

        auto& buses = db_.GetAllBuses();
        for(auto& [name, bus] : buses) {
            for(size_t i = 0; i < bus.route_.size(); ++i) {
                size_t vertexId1 = vertexIds_[bus.route_[i]];
                for(size_t j = i + 1; j < bus.route_.size(); ++j) {
                    auto distance = db_.ComputeRealStopToStopDistance(bus, i, j) / 1000 / (routingSetting_.busVelocity / 60);
                    size_t vertexId2 = vertexIds_[bus.route_[j]];
                    size_t edgeId = graph_.AddEdge({vertexId1 + 1, vertexId2, distance});
                    edgeIds_[edgeId] = {false,distance,nullptr,&bus,(int)j - (int)i};
                }
            }
        }
        return graph_;
    }

    std::optional<RouteInfo> TransportRouter::GetOptimalRoute(std::string& routeFrom, std::string& routeTo) const {
        size_t idFrom = vertexIds_.at(&db_.GetStop(routeFrom));
        size_t idTo = vertexIds_.at(&db_.GetStop(routeTo));
        auto graphRouteInfo = router_.BuildRoute(idFrom, idTo);

        if(graphRouteInfo.has_value()) {
            transport_router::RouteInfo completeRouteInfo;
            completeRouteInfo.totalTime = graphRouteInfo.value().weight;
            for(auto edgeId : graphRouteInfo.value().edges) {
                completeRouteInfo.routeSteps.push_back(edgeIds_.at(edgeId));
            }
            return completeRouteInfo;
        }
        return {};

    }

}

