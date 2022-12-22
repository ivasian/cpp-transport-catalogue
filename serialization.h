#pragma once
#include "transport_catalogue.pb.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include <fstream>



namespace serialization {

        void Serialize(std::ostream& output,
                       const transport_catalogue::TransportCatalogue& catalogue,
                       const renderer::MapRenderer& mapRenderer,
                       const transport_router::TransportRouter& transportRouter);
        void Deserialize(std::istream& input,
                         transport_catalogue::TransportCatalogue& outCatalogue,
                         renderer::MapRenderer& outMapRenderer,
                         transport_router::TransportRouter& transportRouter);

        [[nodiscard]] serialization::RenderSettings Convert(const renderer::RenderSettings& settings);
        [[nodiscard]] serialization::Graph Convert(const transport_router::Graph& graph);
        [[nodiscard]] serialization::TransportCatalogue Convert(const transport_catalogue::TransportCatalogue& catalogue);
        [[nodiscard]] serialization::RoutingSettings Convert(const transport_router::RoutingSetting& settings);
        [[nodiscard]] serialization::Transport_router Convert(const transport_router::TransportRouter& router);
        [[nodiscard]] serialization::Map_renderer Convert(const renderer::MapRenderer& map);


        [[nodiscard]] transport_router::RoutingSetting Convert(const serialization::RoutingSettings& settings);
        [[nodiscard]] transport_catalogue::TransportCatalogue Convert(const serialization::TransportCatalogue& catalogue);
        [[nodiscard]] transport_router::Graph Convert(const serialization::Graph& graph);
        [[nodiscard]] renderer::RenderSettings Convert(const serialization::RenderSettings& settings);
        [[nodiscard]] transport_router::TransportRouter Convert(const serialization::Transport_router& router, const transport_catalogue::TransportCatalogue& catalogue);
        [[nodiscard]] renderer::MapRenderer Convert(const serialization::Map_renderer& map);


}