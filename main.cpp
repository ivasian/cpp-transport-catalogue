#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "router.h"
#include "transport_router.h"
#include "serialization.h"
/*
int main() {
    using transport_catalogue::TransportCatalogue;
    using renderer::RenderSettings;
    using transport_router::RoutingSetting;
    using transport_router::TransportRouter;

    JsonReader jsonReader;
    json::Document doc = json::Load(std::cin);
    TransportCatalogue catalogue = jsonReader.BuildCatalogueBase(doc);
    RenderSettings mapRenderer = jsonReader.GetMapRenderSettings(doc);
    RoutingSetting routingSetting = jsonReader.LoadRoutingSettings(doc);
    TransportRouter transportRouter(catalogue, routingSetting);


    RequestHandler requestHandler(catalogue, mapRenderer, transportRouter);

    json::Document result = requestHandler.ExecuteQuery(doc);
    Print(result, std::cout);

}
*/

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        JsonReader jsonReader;
        json::Document doc = json::Load(std::cin);
        TransportCatalogue catalogue = jsonReader.BuildCatalogueBase(doc);
        SerializationSetting serializationSetting = jsonReader.LoadSerializationSettings(doc);
        serialization::TransportCatalogue catalogSerialization(serializationSetting.filename);
        RenderSettings mapRenderer = jsonReader.GetMapRenderSettings(doc);
        RoutingSetting routingSetting = jsonReader.LoadRoutingSettings(doc);
        TransportRouter transportRouter(catalogue, routingSetting);

        catalogSerialization.Serialize(catalogue, mapRenderer, routingSetting, transportRouter.GetGraph());

    } else if (mode == "process_requests"sv) {
        JsonReader jsonReader;
        json::Document doc = json::Load(std::cin);
        SerializationSetting serializationSetting = jsonReader.LoadSerializationSettings(doc);
        serialization::TransportCatalogue catalogSerialization(serializationSetting.filename);
        TransportCatalogue catalogue;
        RenderSettings mapRenderer;
        transport_router::Graph graph;
        RoutingSetting routingSetting;
        catalogSerialization.Deserialize(catalogue, mapRenderer, routingSetting, graph);


        TransportRouter transportRouter(catalogue, routingSetting);
        RequestHandler requestHandler(catalogue, mapRenderer, transportRouter);

        json::Document result = requestHandler.ExecuteQuery(doc);
        Print(result, std::cout);
        // process requests here

    } else {
        PrintUsage();
        return 1;
    }
}