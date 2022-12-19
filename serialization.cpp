#include "serialization.h"

namespace serialization {


    void TransportCatalogue::Serialize(const transport_catalogue::TransportCatalogue& catalogue,
                                       const renderer::RenderSettings& renderSettings,
                                       const transport_router::RoutingSetting& routingSettings,
                                       const transport_router::Graph & graph) {
        std::ofstream output(filename_, std::ios::binary);
        transportCatalogueSerialize::Base base;
        transportCatalogueSerialize::TransportCatalogue outCatalogue;
        transportCatalogueSerialize::RenderSettings outRenderSettings;
        transportCatalogueSerialize::RoutingSettings outRoutingSettings;
        transportCatalogueSerialize::Graph outGraph;
        Convert(catalogue, outCatalogue);
        Convert(renderSettings, outRenderSettings);
        Convert(routingSettings, outRoutingSettings);
        Convert(graph, outGraph);
        *base.mutable_graph() = outGraph;
        *base.mutable_catalogue() = outCatalogue;
        *base.mutable_rendersettings() = outRenderSettings;
        *base.mutable_routingsettings() = outRoutingSettings;
        base.SerializeToOstream(&output);
    }

    void TransportCatalogue::Deserialize(transport_catalogue::TransportCatalogue& outCatalogue,
                                         renderer::RenderSettings& outRenderSettings,
                                         transport_router::RoutingSetting& outRoutingSettings,
                                         transport_router::Graph& outGraph) {
        std::ifstream input(filename_, std::ios::binary);
        transportCatalogueSerialize::Base base;
        base.ParseFromIstream(&input);
        Convert(*base.mutable_catalogue(), outCatalogue);
        Convert(*base.mutable_rendersettings(), outRenderSettings);
        Convert(*base.mutable_routingsettings(), outRoutingSettings);
        Convert(*base.mutable_graph(), outGraph);
    }

    void TransportCatalogue::Convert(const transport_router::Graph& graph,
                                     transportCatalogueSerialize::Graph& outGraph) {
        const auto& edges = graph.GetEdges();
        for(const auto& edge : edges) {
            transportCatalogueSerialize::Edge toSerialEdge;
            toSerialEdge.set_from(edge.from);
            toSerialEdge.set_to(edge.to);
            toSerialEdge.set_weight(edge.weight);
            *outGraph.add_edges() = toSerialEdge;
        }

        const auto& incidenceLists = graph.GetIncidenceLists();
        for(const auto& incidenceList : incidenceLists) {
            auto toSerialIncidenceList = outGraph.add_incidence_lists();
            for(const auto& edgeId : incidenceList) {
                toSerialIncidenceList->add_list(edgeId);
            }
        }
    }

    void TransportCatalogue::Convert(const transportCatalogueSerialize::Graph& graph,
                                     transport_router::Graph& OutGraph) {
        std::vector<graph::Edge<double>> edges;
        for(size_t i = 0; i < graph.edges_size(); ++i) {
            edges.push_back({graph.edges(i).from(), graph.edges(i).to(), graph.edges(i).weight()});
        }
        std::vector<std::vector<size_t>> incidenceLists;
        for(size_t i = 0; i < graph.incidence_lists_size(); ++i) {
            const auto& incidenceList = graph.incidence_lists(i);
            incidenceLists.push_back({});
            for(size_t j = 0; j < incidenceList.list_size(); ++j) {
                incidenceLists[i].push_back(incidenceList.list(j));
            }
        }
        OutGraph.SetGraph(edges, incidenceLists);
    }

    void TransportCatalogue::Convert(const transport_catalogue::TransportCatalogue& catalogue,
                                     transportCatalogueSerialize::TransportCatalogue& outCatalogue) {
        const auto& stops = catalogue.GetAllStops();
        std::unordered_map<const transport_catalogue::Stop*, size_t> stopToId;
        for(size_t i = 0; i < stops.size(); ++i) {
            transportCatalogueSerialize::Stop tempStop;
            tempStop.set_name(stops[i].name_);
            tempStop.mutable_coordinates()->set_lat(stops[i].coordinates_.lat);
            tempStop.mutable_coordinates()->set_lng(stops[i].coordinates_.lng);
            outCatalogue.add_stops();
            *(outCatalogue.mutable_stops(i)) = tempStop;
            stopToId.emplace(&stops[i], i);
        }

        const auto& buses = catalogue.GetAllBuses();
        for(const auto&[name, bus] : buses) {
            transportCatalogueSerialize::Bus tempBus;
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
            transportCatalogueSerialize::StopDistance tempStopDistance;
            tempStopDistance.set_stop1(stopToId.at(stopPair.first));
            tempStopDistance.set_stop2(stopToId.at(stopPair.second));
            tempStopDistance.set_distance(distance);
            outCatalogue.add_stopdistances();
            *(outCatalogue.mutable_stopdistances(outCatalogue.stopdistances_size() -1 )) = tempStopDistance;
        }
    }
    void TransportCatalogue::Convert(const transportCatalogueSerialize::TransportCatalogue& catalogue, transport_catalogue::TransportCatalogue& outCatalogue) {
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
    }

    void TransportCatalogue::Convert(const renderer::RenderSettings& settings, transportCatalogueSerialize::RenderSettings& outSettings) {
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
    }

    void TransportCatalogue::Convert(const transport_router::RoutingSetting& settings, transportCatalogueSerialize::RoutingSettings& outSettings) {
        outSettings.set_buswaittime(settings.busWaitTime);
        outSettings.set_busvelocity(settings.busVelocity);
    }

    void TransportCatalogue::Convert(const transportCatalogueSerialize::RoutingSettings& settings, transport_router::RoutingSetting& outSettings) {
        outSettings.busWaitTime = settings.buswaittime();
        outSettings.busVelocity = settings.busvelocity();
    }

    void TransportCatalogue::Convert(const transportCatalogueSerialize::RenderSettings& settings, renderer::RenderSettings& outSettings) {
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
    }
}