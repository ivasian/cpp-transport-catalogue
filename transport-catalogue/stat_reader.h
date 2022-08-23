#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <cassert>
#include <iomanip>
#include <sstream>

#include "transport_catalogue.h"

namespace transport_catalogue::detail {
    class OutQueries {

    public:
        struct BusInfoQuery {
            std::string name_;
        };

        struct StopInfoQuery {
            std::string name_;
        };

        OutQueries(TransportCatalogue& transportCatalogue, std::istream& in, std::ostream& out) :
                transportCatalogue_(transportCatalogue), in_(in), out_(out){
        }

        void readInput();

        BusInfoQuery parseBusInfoQuery(std::string& busInfoQuery);
        StopInfoQuery parseStopInfoQuery(std::string& stopInfoQuery);

        void executeStopInfoQuery(StopInfoQuery query);
        void executeBusInfoQuery(BusInfoQuery query);

        const std::vector<BusInfoQuery>& GetBusInfoQueries() const;

            private:
        std::vector<BusInfoQuery> busInfoQueries_;
        std::vector<BusInfoQuery> stopInfoQueries_;
        TransportCatalogue& transportCatalogue_;
        std::istream& in_;
        std::ostream& out_;
    };

    namespace tests{
        void OutQueriesFromCatalogue(TransportCatalogue& catalogue);
    }
}