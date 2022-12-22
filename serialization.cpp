#include "serialization.h"

namespace serialization {

    void Serialize(std::ostream& output,
                   const transport_catalogue::TransportCatalogue& catalogue,
                   const renderer::MapRenderer& mapRenderer,
                   const transport_router::TransportRouter& router) {

        serialization::Base base;
        *base.mutable_router() = Convert(router);
        *base.mutable_catalogue() = Convert(catalogue);
        *base.mutable_renderer() = Convert(mapRenderer);
        base.SerializeToOstream(&output);

    }

    void Deserialize(std::istream& input,
                     transport_catalogue::TransportCatalogue& outCatalogue,
                     renderer::MapRenderer& outMapRenderer,
                     transport_router::TransportRouter& outRouter) {

        serialization::Base base;
        base.ParseFromIstream(&input);
        outCatalogue = Convert(base.catalogue());
        outMapRenderer = Convert(base.renderer());
        outRouter = Convert(base.router(), outCatalogue);
    }

    serialization::Graph Convert(const transport_router::Graph& graph) {
        serialization::Graph outGraph;
        const auto& edges = graph.GetEdges();
        for(const auto& edge : edges) {
            serialization::Edge toSerialEdge;
            toSerialEdge.set_from(edge.from);
            toSerialEdge.set_to(edge.to);
            toSerialEdge.set_weight(edge.weight);
            outGraph.mutable_edges()->Add(std::move(toSerialEdge));
        }

        const auto& incidenceLists = graph.GetIncidenceLists();
        for(const auto& incidenceList : incidenceLists) {
            serialization::IncidenceList toSerialIncidenceList;
            for(const auto& edgeId : incidenceList) {
                toSerialIncidenceList.add_list(edgeId);
            }
            outGraph.mutable_incidence_lists()->Add(std::move(toSerialIncidenceList));
        }
        return outGraph;
    }

    transport_router::Graph Convert(const serialization::Graph& graph) {
        transport_router::Graph outGraph;
        std::vector<graph::Edge<double>> edges;
        for(size_t i = 0; i < graph.edges_size(); ++i) {
            edges.push_back({graph.edges(i).from(), graph.edges(i).to(), graph.edges(i).weight()});
        }
        std::vector<std::vector<size_t>> incidenceLists;
        for(size_t i = 0; i < graph.incidence_lists_size(); ++i) {
            const auto& incidenceList = graph.incidence_lists(i);
            incidenceLists.emplace_back();
            for(size_t j = 0; j < incidenceList.list_size(); ++j) {
                incidenceLists[i].push_back(incidenceList.list(j));
            }
        }
        outGraph.SetGraph(edges, incidenceLists);
        return outGraph;
    }

    serialization::TransportCatalogue Convert(const transport_catalogue::TransportCatalogue& catalogue) {
        serialization::TransportCatalogue outCatalogue;
        const auto& stops = catalogue.GetAllStops();
        std::unordered_map<const transport_catalogue::Stop*, size_t> stopToId;
        for(size_t i = 0; i < stops.size(); ++i) {
            serialization::Stop tempStop;
            tempStop.set_name(stops[i].name_);
            tempStop.mutable_coordinates()->set_lat(stops[i].coordinates_.lat);
            tempStop.mutable_coordinates()->set_lng(stops[i].coordinates_.lng);
            outCatalogue.add_stops();
            *(outCatalogue.mutable_stops(i)) = tempStop;
            stopToId.emplace(&stops[i], i);
        }

        const auto& buses = catalogue.GetBuses();
        for(const auto& bus : buses) {
            serialization::Bus tempBus;
            tempBus.set_name(bus.name_);
            for(const auto* stopPtr : bus.route_) {
                tempBus.add_route(stopToId.at(stopPtr));
            }
            tempBus.set_isroundtrip(bus.isRoundtrip_);
            outCatalogue.add_buses();
            *(outCatalogue.mutable_buses(outCatalogue.buses_size() - 1)) = tempBus;
        }

        const auto& stopDistances = catalogue.GetStopDistances();
        for(const auto& [stopPair, distance ] : stopDistances) {
            serialization::StopDistance tempStopDistance;
            tempStopDistance.set_stop1(stopToId.at(stopPair.first));
            tempStopDistance.set_stop2(stopToId.at(stopPair.second));
            tempStopDistance.set_distance(distance);
            outCatalogue.add_stopdistances();
            *(outCatalogue.mutable_stopdistances(outCatalogue.stopdistances_size() -1 )) = tempStopDistance;
        }
        return outCatalogue;
    }

    transport_catalogue::TransportCatalogue Convert(const serialization::TransportCatalogue& catalogue) {
        transport_catalogue::TransportCatalogue outCatalogue;
        for(size_t i = 0; i < catalogue.stops_size(); ++i) {
            const auto deserStop = catalogue.stops(i);
            outCatalogue.AddStop({deserStop.name(),
                               {deserStop.coordinates().lat(),
                                          deserStop.coordinates().lng()}});
        }
        for(size_t i = 0; i < catalogue.buses_size(); ++i) {
            const auto& deserBus = catalogue.buses(i);
            std::vector<const transport_catalogue::Stop*> stops;
            for(size_t j = 0; j < deserBus.route_size(); ++j) {
                const auto stopId = deserBus.route(j);
                const auto& stopName = catalogue.stops(stopId).name();
                stops.push_back(&outCatalogue.GetStop(stopName));
            }
            outCatalogue.AddBus({deserBus.name(), stops, deserBus.isroundtrip()});
        }
        for(size_t i = 0; i < catalogue.stopdistances_size(); ++i) {
            const auto& stopDistance = catalogue.stopdistances(i);
            const auto& stopName1 = catalogue.stops(stopDistance.stop1()).name();
            const auto& stopName2 = catalogue.stops(stopDistance.stop2()).name();
            outCatalogue.SetStopsDistance(&outCatalogue.GetStop(stopName1),
                                       &outCatalogue.GetStop(stopName2),
                                       stopDistance.distance());
        }
        return outCatalogue;
    }

    serialization::RenderSettings Convert(const renderer::RenderSettings& settings) {
        serialization::RenderSettings outSettings;
        outSettings.set_width(settings.width_);
        outSettings.set_height(settings.height_);
        outSettings.set_padding(settings.padding_);
        outSettings.set_line_width(settings.line_width_);
        outSettings.set_stop_radius(settings.stop_radius_);
        outSettings.set_bus_label_font_size(settings.bus_label_font_size_);
        outSettings.set_bus_label_offset_1(settings.bus_label_offset_.first);
        outSettings.set_bus_label_offset_2(settings.bus_label_offset_.second);
        outSettings.set_stop_label_font_size(settings.stop_label_font_size_);
        outSettings.set_stop_label_offset_1(settings.stop_label_offset_.first);
        outSettings.set_stop_label_offset_2(settings.stop_label_offset_.second);
        outSettings.set_underlayer_color(settings.underlayer_color_);
        outSettings.set_underlayer_width(settings.underlayer_width_);;
        for(const auto& color : settings.color_palette_) {
            outSettings.add_color_palette(color);
        }
        return outSettings;
    }

    serialization::RoutingSettings Convert(const transport_router::RoutingSetting& settings) {
        serialization::RoutingSettings outSettings;
        outSettings.set_buswaittime(settings.busWaitTime);
        outSettings.set_busvelocity(settings.busVelocity);
        return outSettings;
    }

    transport_router::RoutingSetting Convert(const serialization::RoutingSettings& settings) {
        return {settings.buswaittime(), settings.busvelocity()};
    }

    renderer::RenderSettings Convert(const serialization::RenderSettings& settings) {
        renderer::RenderSettings outSettings;
        outSettings.width_ = settings.width();
        outSettings.height_ = settings.height();
        outSettings.padding_ = settings.padding();
        outSettings.line_width_ = settings.line_width();
        outSettings.stop_radius_ = settings.stop_radius();
        outSettings.bus_label_font_size_ = settings.bus_label_font_size();
        outSettings.bus_label_offset_ = {settings.bus_label_offset_1(), settings.bus_label_offset_2()};
        outSettings.stop_label_font_size_ = settings.stop_label_font_size();
        outSettings.stop_label_offset_ = {settings.stop_label_offset_1(), settings.stop_label_offset_2()};
        outSettings.underlayer_color_ = settings.underlayer_color();
        outSettings.underlayer_width_ = settings.underlayer_width();
        for(size_t i = 0; i < settings.color_palette_size(); ++i) {
            outSettings.color_palette_.push_back(settings.color_palette(i));
        }
        return outSettings;
    }


    renderer::MapRenderer Convert(const serialization::Map_renderer& map) {
        return {Convert(map.settings())};
    }

    serialization::Map_renderer Convert(const renderer::MapRenderer& map) {
        serialization::RenderSettings outRenderSetting = Convert(map.GetRenderSettings());
        serialization::Map_renderer mapRenderer;
        *mapRenderer.mutable_settings() = outRenderSetting;
        return mapRenderer;
    }


    serialization::Transport_router Convert(const transport_router::TransportRouter& router) {
        serialization::Transport_router outRouter;
        *outRouter.mutable_settings() = Convert(router.GetRoutingSetting());
        *outRouter.mutable_graph() = Convert(router.GetGraph());
        return outRouter;
    }

    transport_router::TransportRouter Convert(const serialization::Transport_router& router, const transport_catalogue::TransportCatalogue& catalogue) {
        return {catalogue, Convert(router.settings()), Convert(router.graph())};
    }

}