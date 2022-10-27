#include "json_reader.h"
#include "json_builder.h"


TransportCatalogue JsonReader::BuildCatalogueBase(const Document& doc){
    TransportCatalogue catalogue;
    LoadBaseRequests(catalogue, doc);
    return catalogue;
}

void JsonReader::LoadBaseRequests(TransportCatalogue& outCatalogue, const Document& doc) {
    using namespace std::literals;
    auto& node = doc.GetRoot();
    auto& baseRequests = node.AsDict().at("base_requests"s).AsArray();
    std::vector<StopQuery> stops;
    std::vector<BusQuery> buses;
    for(auto& elem : baseRequests) {
        if(elem.AsDict().at("type"s) == "Stop"s) {
            auto& stopNode = elem.AsDict();
            stops.push_back({stopNode.at("name"s).AsString(), stopNode.at("latitude"s).AsDouble(),
                             stopNode.at("longitude"s).AsDouble(), GetDistanceToStops(stopNode.at("road_distances"s))});
        } else if(elem.AsDict().at("type"s) == "Bus"s) {
            auto& busNode = elem.AsDict();
            bool isRingRoute = busNode.at("is_roundtrip"s).AsBool();
            auto busStops = GetStopNamesInRoute(busNode.at("stops"s));
            if(!isRingRoute) {
                for(int i = busStops.size() - 2; i >= 0; --i) {
                    busStops.push_back(busStops[i]);
                }
            }
            buses.push_back({busNode.at("name"s).AsString(), busStops, isRingRoute});
        } else {
            assert(elem.AsDict().at("type"s) == "Stop"s || elem.AsDict().at("type"s) == "Bus"s);
        }
    }
    LoadDataInCatalogue(outCatalogue, stops, buses);
}

RoutingSetting JsonReader::LoadRoutingSettings(const Document& doc) {
    using namespace std::literals;
    auto& node = doc.GetRoot();
    auto& routingSettings = node.AsDict().at("routing_settings"s).AsDict();
    return {routingSettings.at("bus_wait_time"s).AsDouble(),
                                     routingSettings.at("bus_velocity"s).AsDouble()};
}

RenderSettings JsonReader::GetMapRenderSettings(const Document& doc){
    using namespace std::literals;
    auto &node = doc.GetRoot();
    auto &renderSettingsNode = node.AsDict().at("render_settings"s).AsDict();
    return {renderSettingsNode.at("width"s).AsDouble(),
            renderSettingsNode.at("height"s).AsDouble(),
            renderSettingsNode.at("padding"s).AsDouble(),
            renderSettingsNode.at("line_width"s).AsDouble(),
            renderSettingsNode.at("stop_radius"s).AsDouble(),
            renderSettingsNode.at("bus_label_font_size"s).AsInt(),
            {renderSettingsNode.at("bus_label_offset"s).AsArray()[0].AsDouble(),
             renderSettingsNode.at("bus_label_offset"s).AsArray()[1].AsDouble()},
            renderSettingsNode.at("stop_label_font_size"s).AsInt(),
            {renderSettingsNode.at("stop_label_offset"s).AsArray()[0].AsDouble(),
             renderSettingsNode.at("stop_label_offset"s).AsArray()[1].AsDouble()},
            GetColorFromNode(renderSettingsNode.at("underlayer_color"s)),
            renderSettingsNode.at("underlayer_width"s).AsDouble(),
            GetArrayColorFromNode(renderSettingsNode.at("color_palette"s).AsArray())};
}

StopsDistancesArray  JsonReader::GetDistanceToStops(const Node& nodeWithStopNamesAndDistance){
    StopsDistancesArray distanceToStops;
    for(auto& [stopName, distance] : nodeWithStopNamesAndDistance.AsDict()){
        distanceToStops.push_back({stopName, distance.AsInt()});
    }
    return distanceToStops;
}

svg::Color JsonReader::GetColorFromNode(const Node& node) {
    using namespace std::literals;
    if(node.IsString()) {
        return node.AsString();
    }
    auto color = node.AsArray();
    if(color.size() == 3) {
        return std::string("rgb("s + std::to_string(color[0].AsInt()) + ","s +
                           std::to_string(color[1].AsInt()) + ","s +
                           std::to_string(color[2].AsInt()) + ")"s);
    } else {
        std::ostringstream buf;
        buf << "rgba("s << color[0].AsInt() << ","s
            << color[1].AsInt() << ","s
            << color[2].AsInt() << ","s
            << color[3].AsDouble() << ")"s;

        return buf.str();
    }
}

std::vector<svg::Color> JsonReader::GetArrayColorFromNode(const Node& node) {
    auto colorArrayNode = node.AsArray();
    std::vector<svg::Color> colorArray;
    for(auto colorNode : colorArrayNode) {
        colorArray.push_back(GetColorFromNode(colorNode));
    }
    return colorArray;
}

std::vector<std::string> JsonReader::GetStopNamesInRoute(const Node& nodeWithStopNames){
    std::vector<std::string> stopNames;
    for(auto& stopNode : nodeWithStopNames.AsArray()) {
        stopNames.push_back(stopNode.AsString());
    }
    return stopNames;
}

void JsonReader::LoadDataInCatalogue(TransportCatalogue& catalogue,
                                     const std::vector<transport_catalogue::StopQuery>& stops,
                                     const std::vector<transport_catalogue::BusQuery>& buses) {
    for(auto stop : stops) {
        catalogue.AddStop({stop.name_, {stop.latitude_, stop.longitude_}});
    }

    for(auto bus : buses) {
        std::vector<const Stop*> route;
        for(std::string& stopName : bus.stopNames_) {
            route.push_back(&catalogue.GetStop(stopName));
        }
        catalogue.AddBus({bus.name_, route, bus.isRoundtrip_});
    }

    for(auto stopFrom : stops) {
        const Stop* stopFromInCatalogue = &catalogue.GetStop(stopFrom.name_);
        for(auto &[stopTo, distance] : stopFrom.distance_to_stops_) {
            const transport_catalogue::Stop* stopToInCatalogue = &catalogue.GetStop(stopTo);
            catalogue.SetStopsDistance(stopFromInCatalogue, stopToInCatalogue, distance);
        }
    }
}