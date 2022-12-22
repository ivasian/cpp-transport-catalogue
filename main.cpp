#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "router.h"
#include "transport_router.h"
#include "serialization.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

void PrintGraph(transport_router::Graph graph) {
    auto edges = graph.GetEdges();
    for(auto edge : edges) {
        std::cout << edge.from << " " << edge.to << " " << edge.weight << " ";
    }
    std::cout << std::endl;
    auto lists = graph.GetIncidenceLists();
    for(auto list : lists) {
        for(auto val : list) {
            std::cout << val << " ";
        }
    }
    std::cout << std::endl;
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
        MapRenderer mapRenderer(jsonReader.GetMapRenderSettings(doc));
        RoutingSetting routingSetting = jsonReader.LoadRoutingSettings(doc);
        TransportRouter router(catalogue, routingSetting);
        {
            std::ofstream output(serializationSetting.filename, std::ios::binary);
            serialization::Serialize(output, catalogue, mapRenderer, router);
        }


    } else if (mode == "process_requests"sv) {
        JsonReader jsonReader;
        json::Document doc = json::Load(std::cin);
        SerializationSetting serializationSetting = jsonReader.LoadSerializationSettings(doc);

        TransportCatalogue catalogue;
        MapRenderer mapRenderer;
        TransportRouter transportRouter;
        {
            std::ifstream input(serializationSetting.filename, std::ios::binary);
            serialization::Deserialize(input, catalogue, mapRenderer, transportRouter);
        }
        RequestHandler requestHandler(catalogue, mapRenderer, transportRouter);
        json::Document result = requestHandler.ExecuteQuery(doc);
        Print(result, std::cout);

    } else {
        PrintUsage();
        return 1;
    }
}