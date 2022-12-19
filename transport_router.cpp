#include "transport_router.h"



namespace transport_router {

    const double TO_DISTANCE_IN_MINUTE = 60/1000;

    TransportRouter::TransportRouter(const TransportCatalogue& db, RoutingSetting routingSetting) :
            db_(db), routingSetting_(routingSetting), router_(BuildGraph()){
    }

    TransportRouter::TransportRouter(const TransportCatalogue& db, RoutingSetting routingSetting, Graph& graph):
            db_(db), routingSetting_(routingSetting), router_(graph){
        FillGraphWithStops(db_.GetAllStops(), true);
        FillGraphWithBuses(db_.GetAllBuses(), true);
    }

    Graph& TransportRouter::BuildGraph() {
        graph_ = Graph(db_.GetAllStops().size() * 2);
        FillGraphWithStops(db_.GetAllStops());
        FillGraphWithBuses(db_.GetAllBuses());
        return graph_;
    }

    void TransportRouter::FillGraphWithStops(const std::deque<Stop>& stops, bool isGraphDeserialized) {
        size_t vertexId = 0;
        for(const auto& stop : stops) {
            size_t edgeId = edgeIds_.size();
            if(!isGraphDeserialized) {
                edgeId = graph_.AddEdge({vertexId, vertexId + 1, routingSetting_.busWaitTime});
            }
            edgeIds_[edgeId] = std::make_shared<OnWait>(routingSetting_.busWaitTime, &stop);
            vertexIds_[&stop] = vertexId;
            vertexId += 2;
        }
    }

    void TransportRouter::FillGraphWithBuses(const BusesContaner& buses, bool isGraphDeserialized) {
        for(const auto& [name, bus] : buses) {
            for(size_t i = 0; i < bus.route_.size(); ++i) {
                size_t vertexId1 = vertexIds_[bus.route_[i]];
                for(size_t j = i + 1; j < bus.route_.size(); ++j) {
                    auto edgeDistance = ComputeTimeInMinute(db_.ComputeRealStopToStopDistance(bus, i, j), routingSetting_.busVelocity);
                    size_t vertexId2 = vertexIds_[bus.route_[j]];
                    size_t edgeId = edgeIds_.size();
                    if(!isGraphDeserialized) {
                        edgeId = graph_.AddEdge({vertexId1 + 1, vertexId2, edgeDistance});
                    }
                    int spanCount = static_cast<int>(j) - static_cast<int>(i);
                    edgeIds_[edgeId] = std::make_shared<OnBus>(edgeDistance, &bus, spanCount );
                }
            }
        }
    }

    std::optional<RouteInfo> TransportRouter::GetOptimalRoute(const std::string& routeFrom, const std::string& routeTo) const {
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

    const std::map<const Stop*, size_t>& TransportRouter::GetVertexIds() const {
        return vertexIds_;
    }

    const std::map<int, std::shared_ptr<Activity>>& TransportRouter::GetEdgeIds() const {
        return edgeIds_;
    }

    const Graph& TransportRouter::GetGraph() const {
        return graph_;
    }

    double TransportRouter::ComputeTimeInMinute (double sInMeters, double vInKmh) const {
        double sInKm = sInMeters / 1000;
        double tInH = sInKm / vInKmh;
        return tInH * 60;
    }



        Activity::Activity(double time) : time_(time) {
        }
        void Activity::WriteInJsonDict(json::Dict& dict) {
            using namespace std::literals;
            dict.insert({"time"s, json::Builder{}.Value(time_).Build()});
        }

        OnWait::OnWait(double time, const Stop* stop) : Activity(time), stop_(stop){
        }
        void OnWait::WriteInJsonDict(json::Dict& dict) {
            using namespace std::literals;

            transport_router::Activity::WriteInJsonDict(dict);
            dict.insert({"type"s, json::Builder{}.Value("Wait"s).Build()});
            dict.insert({"stop_name"s, json::Builder{}.Value(std::string(stop_->name_)).Build()});
        }

        OnBus::OnBus(double time, const Bus* bus, int spanCount) : Activity(time), bus_(bus), spanCount_(spanCount){
        }
        void OnBus::WriteInJsonDict(json::Dict& dict) {
            using namespace std::literals;
            transport_router::Activity::WriteInJsonDict(dict);
            dict.insert({"type"s, json::Builder{}.Value("Bus"s).Build()});
            dict.insert({"bus"s, json::Builder{}.Value(std::string(bus_->name_)).Build()});
            dict.insert({"span_count"s, json::Builder{}.Value(spanCount_).Build()});
        }
}

