#pragma once
#include "transport_catalogue.pb.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include <fstream>



namespace serialization {

    class TransportCatalogue {
    public:
        TransportCatalogue(std::string filename) : filename_(filename) {
        }

        void Serialize(const transport_catalogue::TransportCatalogue& catalogue,
                       const renderer::RenderSettings& settings,
                       const transport_router::RoutingSetting& outRoutingSettings,
                       const transport_router::Graph & graph);
        void Deserialize(transport_catalogue::TransportCatalogue& InCatalogue,
                         renderer::RenderSettings& InRenderSettings,
                         transport_router::RoutingSetting& InRoutingSettings,
                         transport_router::Graph& graph);

    private:

        void Convert(const renderer::RenderSettings& settings, transportCatalogueSerialize::RenderSettings& OutSettings);
        void Convert(const transport_router::Graph& graph, transportCatalogueSerialize::Graph& outGraph);
        void Convert(const transport_catalogue::TransportCatalogue& catalogue, transportCatalogueSerialize::TransportCatalogue& OutCatalogue);
        void Convert(const transport_router::RoutingSetting& settings, transportCatalogueSerialize::RoutingSettings& outSettings);

        void Convert(const transportCatalogueSerialize::RoutingSettings& settings, transport_router::RoutingSetting& outSettings);
        void Convert(const transportCatalogueSerialize::TransportCatalogue& catalogue, transport_catalogue::TransportCatalogue& OutCatalogue);
        void Convert(const transportCatalogueSerialize::Graph& graph, transport_router::Graph& OutGraph);
        void Convert(const transportCatalogueSerialize::RenderSettings& settings, renderer::RenderSettings& OutSettings);

        std::string filename_;
    };


}