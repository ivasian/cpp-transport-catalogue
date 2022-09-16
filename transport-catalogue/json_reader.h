#pragma once
#include <vector>

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"

using transport_catalogue::TransportCatalogue;
using transport_catalogue::StopQuery;
using transport_catalogue::BusQuery;
using transport_catalogue::Stop;
using transport_catalogue::Bus;
using renderer::RenderSettings;
using json::Document;
using json::Node;
using StopsDistancesArray = std::vector<std::pair<std::string, int>>;

class JsonReader {

public:
    TransportCatalogue BuildCatalogueBase(const Document& doc);
    RenderSettings GetMapRenderSettings(const Document& doc);

private:
    svg::Color GetColorFromNode(const Node& node);
    std::vector<svg::Color> GetArrayColorFromNode(const Node& node);
    StopsDistancesArray  GetDistanceToStops(const Node& nodeWithStopNamesAndDistance);
    std::vector<std::string> GetStopNamesInRoute(const Node& nodeWithStopNames);


    void LoadDataInCatalogue(TransportCatalogue& catalogue,
                             const std::vector<StopQuery>& stops,
                             const std::vector<BusQuery>& buses);
};