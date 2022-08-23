#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <cassert>

#include "transport_catalogue.h"

namespace transport_catalogue::detail {

    class InQueries {
    public:
        void readInput(std::istream& input);
        void executeQueries(TransportCatalogue& catalogue);

        struct StopQuery {
            std::string name_;
            double latitude;
            double longitude;
            std::vector<std::pair<std::string, int>> distance_to_stops;
        };

        struct BusQuery {
            std::string name_;
            std::vector<std::string> stopNames;
        };
    private:
        StopQuery parseStop(std::string& stopQuery);
        BusQuery parseBus(std::string& busQuery);

        std::vector<StopQuery> stops;
        std::vector<BusQuery> buses;
    };

    namespace tests {
        void InQueriesToCatalogue(TransportCatalogue& catalogue);
    }
}