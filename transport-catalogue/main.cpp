#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "router.h"
#include "transport_router.h"


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