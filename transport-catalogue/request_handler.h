#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"
#include "router.h"
#include "transport_router.h"

using transport_catalogue::TransportCatalogue;
using transport_catalogue::Stop;
using transport_catalogue::Bus;
using transport_catalogue::StopQuery;
using transport_catalogue::BusQuery;
using transport_router::TransportRouter;
using renderer::MapRenderer;
using renderer::SphereProjector;
using geo::Coordinates;

using StopsNamesAndCoordinates = std::vector<std::pair<std::string_view, std::vector<Coordinates>>>;
using AllStopsOnBusesByOrder = std::map<std::string_view,const Stop*>;

using Router = graph::Router<double>;

class RequestHandler {

public:

    RequestHandler(const TransportCatalogue& db, const MapRenderer& renderer, const TransportRouter& routeBuilder);

    void RenderMap(std::ostream& out) const;
    void RenderLines(svg::Document& doc, SphereProjector& sphereProjector) const;
    void RenderBusesNames(svg::Document& doc, SphereProjector& sphereProjector) const;
    void RenderStopsIcons(svg::Document& doc, SphereProjector& sphereProjector) const;
    void RenderStopsNames(svg::Document& doc, SphereProjector& sphereProjector) const;
    json::Document ExecuteQuery(const json::Document& doc) const;


private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& db_;
    const MapRenderer& renderer_;
    const TransportRouter& routeBuilder_;

    std::vector<const Stop*> GetStopsForRenderBusName(std::string_view busName) const;
    std::set<std::string_view> GetBusesNamesByOrder() const;
    StopsNamesAndCoordinates GetAllBusesPoints() const;
    std::vector<geo::Coordinates> GetAllStopsCoordinates() const;
    AllStopsOnBusesByOrder GetAllStopsOnBusesByOrder() const;

    void ExecuteStopQuery(json::Dict& outDict, const json::Node& request) const;
    void ExecuteBusQuery(json::Dict& outDict, const json::Node& request) const;
    void ExecuteMapQuery(json::Dict& outDict) const;
    void ExecuteRouteQuery(json::Dict& outDict, const json::Node& request) const;

};