#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <cassert>
#include "transport_catalogue.h"
#include <iomanip>
#include <sstream>

namespace transport_catalogue::detail{
    class OutQueries{

    public:
        struct BusInfoQuery{
            std::string name_;
        };

        struct StopInfoQuery{
            std::string name_;
        };

        OutQueries(TransportCatalogue& transportCatalogue, std::istream& in, std::ostream& out) :
                transportCatalogue_(transportCatalogue), in_(in), out_(out){
        }

        void readInput();
        void writeAnswer();

        BusInfoQuery parseBusInfoQuery(std::string& busInfoQuery);


        const std::vector<BusInfoQuery>& GetBusInfoQueries() const;
        void executeQueries(TransportCatalogue& catalogue, std::ostream& out);
        StopInfoQuery parseStopInfoQuery(std::string& stopInfoQuery);
        void executeStopInfoQuery(StopInfoQuery query);
        void executeBusInfoQuery(BusInfoQuery query);
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