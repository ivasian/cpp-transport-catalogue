#include <iostream>
#include <sstream>
#include "transport_catalogue.h"
#include "stat_reader.h"
#include "input_reader.h"



void MainProgramExecution(){
    using namespace transport_catalogue;
    TransportCatalogue catalogue;
    detail::InQueries inQueries;
    inQueries.readInput(std::cin);
    inQueries.executeQueries(catalogue);

    detail::OutQueries outQueries(catalogue, std::cin, std::cout);
    outQueries.readInput();
}

void TestingExecution(){
    using namespace transport_catalogue;
    {
        TransportCatalogue catalogue;
        detail::tests::InQueriesToCatalogue(catalogue);
    }

    {
        TransportCatalogue catalogue;
        tests::AddingStop(catalogue);
    }
    {
        TransportCatalogue catalogue;
        tests::AddingBus(catalogue);
    }

    {
        TransportCatalogue catalogue;
        tests::GettingBusInfo(catalogue);
    }


}


int main() {
    MainProgramExecution();
    //TestingExecution();
    return 0;
}
