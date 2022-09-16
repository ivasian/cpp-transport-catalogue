#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"


int main() {

    JsonReader jsonReader;
    json::Document doc = json::Load(std::cin);
    transport_catalogue::TransportCatalogue catalogue = jsonReader.BuildCatalogueBase(doc);
    renderer::RenderSettings mapRenderer = jsonReader.GetMapRenderSettings(doc);

    RequestHandler requestHandler(catalogue, mapRenderer);

    json::Document result = requestHandler.ExecuteQuery(doc);
    Print(result, std::cout);
}