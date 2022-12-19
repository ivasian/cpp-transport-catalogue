#include "request_handler.h"
#include "json_builder.h"

RequestHandler::RequestHandler(const TransportCatalogue& db,
                               const MapRenderer& renderer,
                               const TransportRouter& routeBuilder) :
        db_(db), renderer_(renderer), routeBuilder_(routeBuilder){
}

std::vector<geo::Coordinates> RequestHandler::GetAllStopsCoordinates() const {
    std::vector<Coordinates> allCoordinates;

    auto buses = db_.GetAllBuses();
    for(auto& [name, bus] : buses) {
        for(auto& stop : bus.route_) {
            allCoordinates.push_back(stop->coordinates_);
        }
    }
    return allCoordinates;
}

std::set<std::string_view> RequestHandler::GetBusesNamesByOrder() const {
    std::set<std::string_view> busesByOrder;
    auto buses = db_.GetAllBuses();

    for(auto &[name, route] : buses) {
        busesByOrder.insert(name);
    }
    return busesByOrder;
}

std::vector<const Stop*> RequestHandler::GetStopsForRenderBusName(std::string_view busName) const {
    std::vector<const Stop*> finalStops;
    auto bus = db_.GetBus(std::string(busName));
    if(!bus.route_.empty()) {
        if(bus.isRoundtrip_) {
            finalStops.push_back(bus.route_[0]);
        } else {
            finalStops.push_back(bus.route_[0]);
            if(bus.route_[0] != bus.route_[bus.route_.size()/2]){
                finalStops.push_back(bus.route_[bus.route_.size()/2]);
            }
        }
    }
    return finalStops;
}

StopsNamesAndCoordinates RequestHandler::GetAllBusesPoints() const {
    StopsNamesAndCoordinates busesPoints;
    std::set<std::string_view> busesByOrder = GetBusesNamesByOrder();
    auto buses = db_.GetAllBuses();

    for(auto& name : busesByOrder) {
        auto& bus = buses.at(name);
        std::vector<Coordinates> busStopPoints;
        for(auto& stop : bus.route_) {
            busStopPoints.push_back(stop->coordinates_);
        }
        busesPoints.push_back({name, busStopPoints});
    }
    return busesPoints;
}

AllStopsOnBusesByOrder RequestHandler::GetAllStopsOnBusesByOrder() const {
    AllStopsOnBusesByOrder stopsOnBusesByOrder;
    auto& allStops = db_.GetAllStops();
    for(auto& stop : allStops) {
        auto busesWithStop = db_.GetStopInfo(std::string(stop.name_));
        if(!busesWithStop.empty()) {
            stopsOnBusesByOrder[stop.name_] = &stop;
        }
    }
    return stopsOnBusesByOrder;
}

void RequestHandler::RenderMap(std::ostream& out) const {
    auto sphereProjector = renderer_.GetSphereProjector(GetAllStopsCoordinates());
    svg::Document doc;
    RenderLines(doc, sphereProjector);
    RenderBusesNames(doc, sphereProjector);
    RenderStopsIcons(doc, sphereProjector);
    RenderStopsNames(doc, sphereProjector);
    doc.Render(out);
}

void RequestHandler::RenderLines(svg::Document& doc, SphereProjector& sphereProjector) const {
    auto busesPoints = GetAllBusesPoints();
    int lineNumber = 0;
    bool isBusEmpty = true;
    for(auto& [busName, busPoints] : busesPoints) {
        svg::Polyline busLine;
        ++lineNumber;
        for(auto& point : busPoints) {
            isBusEmpty = false;
            busLine.AddPoint(sphereProjector(point));
        }
        if(!isBusEmpty) {
            renderer_.SetLineProperties(busLine, lineNumber);
            doc.Add(busLine);
        }
    }
}

void RequestHandler::RenderBusesNames(svg::Document& doc, SphereProjector& sphereProjector) const {
    const auto busesNamesByOrder = GetBusesNamesByOrder();
    int colorNumber = 0;
    for(const auto& busName : busesNamesByOrder) {
        auto stopsForRenderBusName = GetStopsForRenderBusName(busName);
        if(stopsForRenderBusName.empty()) { continue;}
        Color color = renderer_.GetStrokeColorByOrder(++colorNumber);

        {
            Text substrate;
            renderer_.SetBusTextSubstrateProperties(
                    substrate, std::string(busName),
                    sphereProjector(stopsForRenderBusName[0]->coordinates_));
            doc.Add(substrate);

            Text title;
            renderer_.SetBusTextTitleProperties(
                    title, std::string(busName), color,
                    sphereProjector(stopsForRenderBusName[0]->coordinates_));
            doc.Add(title);
        }
        {
            if(stopsForRenderBusName.size() == 2) {
                Text substrate;
                renderer_.SetBusTextSubstrateProperties(
                        substrate, std::string(busName),
                        sphereProjector(stopsForRenderBusName[1]->coordinates_));
                doc.Add(substrate);

                Text title;
                renderer_.SetBusTextTitleProperties(
                        title, std::string(busName), color,
                        sphereProjector(stopsForRenderBusName[1]->coordinates_));
                doc.Add(title);
            }
        }



    }
}

void RequestHandler::RenderStopsIcons(svg::Document& doc, SphereProjector& sphereProjector) const {
    const auto allStopOnBusesByOrder = GetAllStopsOnBusesByOrder();
    for(const auto& [stopName, stopPtr] : allStopOnBusesByOrder) {
        svg::Circle circle;
        renderer_.SetStopsIconsProperties(circle, sphereProjector(stopPtr->coordinates_));
        doc.Add(circle);
    }
}

void RequestHandler::RenderStopsNames(svg::Document& doc, renderer::SphereProjector& sphereProjector) const {
    const auto allStopOnBusesByOrder = GetAllStopsOnBusesByOrder();
    for(const auto& [stopName, stopPtr] : allStopOnBusesByOrder) {
        {
            svg::Text text;
            renderer_.SetStopsTextSubstrateProperties(text, std::string(stopName), sphereProjector(stopPtr->coordinates_));
            doc.Add(text);
        }
        {
            svg::Text text;
            renderer_.SetStopsTextTitleProperties(text, std::string(stopName), sphereProjector(stopPtr->coordinates_));
            doc.Add(text);
        }
    }
}

void RequestHandler::ExecuteStopQuery(json::Dict& outDict, const json::Node& request) const {
    using namespace std::literals;
    std::string stopName = request.AsDict().at("name"s).AsString();
    json::Array buses;
    for (const auto &bus: db_.GetStopInfo(stopName)) {
        buses.push_back(json::Builder{}.Value(std::string(bus)).Build());
    }
    outDict.insert({"buses"s, buses});
}

void RequestHandler::ExecuteBusQuery(json::Dict& outDict, const json::Node& request) const {
    using namespace std::literals;
    std::string busName = request.AsDict().at("name"s).AsString();
    auto busInfo = db_.GetBusInfo(busName);
    outDict.insert({"curvature"s, json::Builder{}.Value(busInfo.curvature_).Build()});
    outDict.insert({"route_length"s, json::Builder{}.Value(busInfo.routeLength_).Build()});
    outDict.insert({"stop_count"s, json::Builder{}.Value((int) busInfo.stopsAmount_).Build()});
    outDict.insert({"unique_stop_count"s, json::Builder{}.Value((int) busInfo.uniqueStopsAmount_).Build()});
}

void RequestHandler::ExecuteMapQuery(json::Dict& outDict) const {
    using namespace std::literals;
    std::ostringstream buf;
    svg::Document doc;
    RenderMap(buf);
    outDict.insert({"map"s, json::Builder{}.Value(buf.str()).Build()});
}

void RequestHandler::ExecuteRouteQuery(json::Dict& outDict, const json::Node& request) const {
    using namespace std::literals;
    std::string routeFrom = request.AsDict().at("from"s).AsString();
    std::string routeTo = request.AsDict().at("to"s).AsString();

    auto optimalRoute = routeBuilder_.GetOptimalRoute(routeFrom, routeTo);
    if (!optimalRoute.has_value()) {
        outDict.insert({"error_message"s, json::Builder{}.Value("not found"s).Build()});
    } else {
        outDict.insert({"total_time"s, json::Builder{}.Value(optimalRoute.value().totalTime).Build()});
        json::Array jsonArray;
        for (auto &routeStep: optimalRoute.value().routeSteps) {
            json::Dict jsonDict;
            routeStep->WriteInJsonDict(jsonDict);
            jsonArray.push_back(json::Builder{}.Value(jsonDict).Build());
        }
        outDict.insert({"items"s, json::Builder{}.Value(jsonArray).Build()});
    }
}

json::Document RequestHandler::ExecuteQuery(const json::Document& doc) const {
    using namespace std::literals;
    auto &node = doc.GetRoot();
    auto &statRequests = node.AsDict().at("stat_requests"s).AsArray();
    json::Array answer;
    for (auto &request: statRequests) {
        int id = request.AsDict().at("id"s).AsInt();
        json::Dict dict = json::Builder{}.StartDict().Key("request_id"s).Value(id).EndDict().Build().AsDict();
        try {
            if (request.AsDict().at("type"s).AsString() == "Stop"s) {
                ExecuteStopQuery(dict, request);
            } else if (request.AsDict().at("type"s).AsString() == "Bus"s) {
                ExecuteBusQuery(dict, request);
            } else if (request.AsDict().at("type"s).AsString() == "Map"s) {
                ExecuteMapQuery(dict);
            }  else if (request.AsDict().at("type"s).AsString() == "Route"s) {
                ExecuteRouteQuery(dict, request);
            } else {
                assert(request.AsDict().at("type"s).AsString() == "Bus"s ||
                       request.AsDict().at("type"s).AsString() == "Stop"s ||
                       request.AsDict().at("type"s).AsString() == "Map"s ||
                       request.AsDict().at("type"s).AsString() == "Route"s);
            }
        }
        catch (...) {
            dict.insert({"error_message"s, json::Builder{}.Value("not found"s).Build()});
        }
        answer.push_back(json::Builder{}.Value(dict).Build());
    }
    return json::Document(answer);
}
